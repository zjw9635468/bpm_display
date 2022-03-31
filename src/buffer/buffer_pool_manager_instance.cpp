//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager_instance.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager_instance.h"

#include "common/macros.h"

namespace bustub {

BufferPoolManagerInstance::BufferPoolManagerInstance(size_t pool_size, DiskManager *disk_manager,
                                                     LogManager *log_manager)
    : BufferPoolManagerInstance(pool_size, 1, 0, disk_manager, log_manager) {}

BufferPoolManagerInstance::BufferPoolManagerInstance(size_t pool_size, uint32_t num_instances, uint32_t instance_index,
                                                     DiskManager *disk_manager, LogManager *log_manager)
    : pool_size_(pool_size),
      num_instances_(num_instances),
      instance_index_(instance_index),
      next_page_id_(instance_index),
      disk_manager_(disk_manager),
      log_manager_(log_manager) {
  BUSTUB_ASSERT(num_instances > 0, "If BPI is not part of a pool, then the pool size should just be 1");
  BUSTUB_ASSERT(
      instance_index < num_instances,
      "BPI index cannot be greater than the number of BPIs in the pool. In non-parallel case, index should just be 1.");
  // We allocate a consecutive memory space for the buffer pool.
  pages_ = new Page[pool_size_];
  replacer_ = new LRUReplacer(pool_size);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}

BufferPoolManagerInstance::~BufferPoolManagerInstance() {
  page_table_.clear();
  free_list_.clear();
  delete[] pages_;
  delete replacer_;
}

bool BufferPoolManagerInstance::FlushPgImp(page_id_t page_id) {
  // Make sure you call DiskManager::WritePage!
  latch_.lock();
  ValidatePageId(page_id);
  if(page_table_.find(page_id) != page_table_.end()){
    frame_id_t index = page_table_[page_id];
    if(pages_[index].is_dirty_){
      disk_manager_->WritePage(page_id,pages_[index].GetData());
      pages_[index].is_dirty_ = false;
    }
    latch_.unlock();
    return true;
  }
  //do nothing if there is no page
  latch_.unlock();
  return false;
}

void BufferPoolManagerInstance::FlushAllPgsImp() {
  // You can do it!
  for(size_t i = 0; i < pool_size_; i++){
    FlushPgImp(page_table_[i]);
  }
}

Page *BufferPoolManagerInstance::NewPgImp(page_id_t *page_id) {
  latch_.lock();
  // 0.   Make sure you call AllocatePage!
  
  // 1.   If all the pages in the buffer pool are pinned, return nullptr.
  
  frame_id_t index;
  
  // 2.   Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
  
  if(!free_list_.empty()){
    index = free_list_.front();
    free_list_.pop_front();
  }else{
    bool has_replacer = replacer_->Victim(&index);
    if(!has_replacer){
      latch_.unlock();
      return nullptr;
    }
  }
  *page_id = AllocatePage();
  if(pages_[index].is_dirty_){
    disk_manager_->WritePage(pages_[index].page_id_,pages_[index].GetData());
    pages_[index].is_dirty_ = false;
  }
    
  page_table_.erase(pages_[index].page_id_);
  
  // 3.   Update P's metadata, zero out memory and add P to the page table.
  pages_[index].page_id_ = *page_id;
  replacer_->Pin(index);
  pages_[index].pin_count_ = 1;
  pages_[index].is_dirty_ = false;
  pages_[index].ResetMemory();
  std::pair<page_id_t, frame_id_t> value(*page_id, index);
  page_table_.insert(value);
  // 4.   Set the page ID output parameter. Return a pointer to P.
  latch_.unlock();
  return &pages_[index];
}

Page *BufferPoolManagerInstance::FetchPgImp(page_id_t page_id) {
  latch_.lock();
  ValidatePageId(page_id);
  frame_id_t index;
  // 1.     Search the page table for the requested page (P).
  if(page_table_.find(page_id) != page_table_.end()){
    // 1.1    If P exists, pin it and return it immediately.
    index = page_table_[page_id];
    replacer_->Pin(index);
    pages_[index].pin_count_++;
    latch_.unlock();
    return &pages_[index];
  }else{
    // 1.2    If P does not exist, find a replacement page (R) from either the free list or the replacer.
    //        Note that pages are always found from the free list first.
    
    if(!free_list_.empty()){
      index = free_list_.front();
      free_list_.pop_front();
    }else{
      bool has_replacer =replacer_->Victim(&index);
      if(free_list_.empty() && !has_replacer){
        //no replacer
        latch_.unlock();
        return nullptr;
      }
    }
    
    // 2.     If R is dirty, write it back to the disk.
    if(pages_[index].is_dirty_){
      disk_manager_->WritePage(pages_[index].page_id_,pages_[index].GetData());
      pages_[index].is_dirty_ = false;
    }
    // 3.     Delete R from the page table and insert P.
    page_table_.erase(pages_[index].page_id_);
    std::pair<page_id_t, frame_id_t> value(page_id, index);
    page_table_.insert(value);
    // 4.     Update P's metadata, read in the page content from disk, and then return a pointer to P.
    pages_[index].ResetMemory();
    disk_manager_->ReadPage(page_id, pages_[index].GetData());
    pages_[index].page_id_ = page_id;
    replacer_->Pin(index);
    pages_[index].pin_count_ = 1;
    pages_[index].is_dirty_ = false;
    
    
    latch_.unlock();
    return &pages_[index];
  }
}

bool BufferPoolManagerInstance::DeletePgImp(page_id_t page_id) {
  latch_.lock();
  ValidatePageId(page_id);
  DeallocatePage(page_id);
  // 0.   Make sure you call DeallocatePage!
  // 1.   Search the page table for the requested page (P).
  if(page_table_.find(page_id) == page_table_.end()){
    // 1.   If P does not exist, return true.
    latch_.unlock();
    return true;
  }else{
    // 2.   If P exists, but has a non-zero pin-count, return false. Someone is using the page.
    frame_id_t index = page_table_[page_id];
    if(pages_[index].pin_count_ != 0){
      latch_.unlock();
      return false;
    }else{
      // 3.   Otherwise, P can be deleted. Remove P from the page table, reset its metadata and return it to the free list.
      if(pages_[index].is_dirty_){
        disk_manager_->WritePage(pages_[index].page_id_,pages_[index].GetData());
      }
      page_table_.erase(pages_[index].page_id_);
      pages_[index].page_id_ = INVALID_PAGE_ID;
      pages_[index].pin_count_ = 0;
      pages_[index].is_dirty_ = false;
      pages_[index].ResetMemory();
      free_list_.push_back(index);
      
      latch_.unlock();
      return true;
    }
  }
  
}

bool BufferPoolManagerInstance::UnpinPgImp(page_id_t page_id, bool is_dirty) {
  latch_.lock();
  ValidatePageId(page_id);
  if(page_table_.find(page_id) != page_table_.end()){
    frame_id_t index = page_table_[page_id];
    //Decrements pin count of the page
    pages_[index].pin_count_--;
    //If the pin count reaches 0, move the associated frame in the LRU
    if(pages_[index].pin_count_ == 0){
      replacer_->Unpin(index);
    }
    //Sets the dirty bit of the page to is_dirty
    //If the bit is already True, don’t set to False
    if(!pages_[index].is_dirty_){
      pages_[index].is_dirty_ = is_dirty;
    }
    latch_.unlock();
    return true;
  }else{
    //not found page_id in page_table_
    latch_.unlock();
    return false;
  }
}

page_id_t BufferPoolManagerInstance::AllocatePage() {
  const page_id_t next_page_id = next_page_id_;
  next_page_id_ += num_instances_;
  ValidatePageId(next_page_id);
  return next_page_id;
}

void BufferPoolManagerInstance::ValidatePageId(const page_id_t page_id) const {
  assert(page_id % num_instances_ == instance_index_);  // allocated pages mod back to this BPI
}

}  // namespace bustub
