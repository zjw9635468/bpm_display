//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// parallel_buffer_pool_manager.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/parallel_buffer_pool_manager.h"

namespace bustub {

ParallelBufferPoolManager::ParallelBufferPoolManager(size_t num_instances, size_t pool_size, DiskManager *disk_manager,
                                                     LogManager *log_manager) {
  // Allocate and create individual BufferPoolManagerInstances
  for(size_t i = 0; i < num_instances; i++){
    BufferPoolManagerInstance* new_buffer_pool = new BufferPoolManagerInstance(pool_size, num_instances, i, disk_manager, log_manager);
    parallel_buffer_pool_manager.push_back(new_buffer_pool);
  }
  last_bpm_index = 0;
  num_of_bpm = num_instances;
}

// Update constructor to destruct all BufferPoolManagerInstances and deallocate any associated memory
ParallelBufferPoolManager::~ParallelBufferPoolManager(){
  parallel_buffer_pool_manager.clear();
  for(size_t i = 0; i < num_of_bpm; i++){
    parallel_buffer_pool_manager[i]->~BufferPoolManagerInstance();
  }
};

size_t ParallelBufferPoolManager::GetPoolSize() {
  // Get size of all BufferPoolManagerInstances
  return num_of_bpm*parallel_buffer_pool_manager[0]->GetPoolSize();
}

BufferPoolManager *ParallelBufferPoolManager::GetBufferPoolManager(page_id_t page_id) {
  // Get BufferPoolManager responsible for handling given page id. You can use this method in your other methods.
  size_t index = page_id % num_of_bpm;

  return parallel_buffer_pool_manager[index];
}

Page *ParallelBufferPoolManager::FetchPgImp(page_id_t page_id) {
  // Fetch page for page_id from responsible BufferPoolManagerInstance
  BufferPoolManager* buffer_pool = GetBufferPoolManager(page_id);
  return buffer_pool->FetchPage(page_id);
  
}

bool ParallelBufferPoolManager::UnpinPgImp(page_id_t page_id, bool is_dirty) {
  // Unpin page_id from responsible BufferPoolManagerInstance
  BufferPoolManager* buffer_pool = GetBufferPoolManager(page_id);
  return buffer_pool->UnpinPage(page_id, is_dirty);
}

bool ParallelBufferPoolManager::FlushPgImp(page_id_t page_id) {
  // Flush page_id from responsible BufferPoolManagerInstance
  BufferPoolManager* buffer_pool = GetBufferPoolManager(page_id);
  return buffer_pool->FlushPage(page_id);
}

Page *ParallelBufferPoolManager::NewPgImp(page_id_t *page_id) {
  // create new page. We will request page allocation in a round robin manner from the underlying
  // BufferPoolManagerInstances
  // 1.   From a starting index of the BPMIs, call NewPageImpl until either 1) success and return 2) looped around to
  // starting index and return nullptr
  // 2.   Bump the starting index (mod number of instances) to start search at a different BPMI each time this function
  // is called
  size_t temp = last_bpm_index;
  Page* return_page = parallel_buffer_pool_manager[last_bpm_index]->NewPage(page_id);
  while(return_page == nullptr){
    last_bpm_index = (last_bpm_index + 1) % num_of_bpm;
    if(last_bpm_index == temp){
      break;
    }
    return_page = parallel_buffer_pool_manager[last_bpm_index]->NewPage(page_id);
  }
  if(return_page != nullptr){
      last_bpm_index = (last_bpm_index + 1) % num_of_bpm;
  }
  return return_page;
}

bool ParallelBufferPoolManager::DeletePgImp(page_id_t page_id) {
  // Delete page_id from responsible BufferPoolManagerInstance
  BufferPoolManager* buffer_pool = GetBufferPoolManager(page_id);
  return buffer_pool->DeletePage(page_id);
}

void ParallelBufferPoolManager::FlushAllPgsImp() {
  // flush all pages from all BufferPoolManagerInstances
  for(size_t i = 0; i < num_of_bpm; i++){
    parallel_buffer_pool_manager[i]->FlushAllPages();
  }
}

}  // namespace bustub
