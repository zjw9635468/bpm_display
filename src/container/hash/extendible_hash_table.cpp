//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// extendible_hash_table.cpp
//
// Identification: src/container/hash/extendible_hash_table.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "common/exception.h"
#include "common/logger.h"
#include "common/rid.h"
#include "container/hash/extendible_hash_table.h"

namespace bustub {

template <typename KeyType, typename ValueType, typename KeyComparator>
HASH_TABLE_TYPE::ExtendibleHashTable(const std::string &name, BufferPoolManager *buffer_pool_manager,
                                     const KeyComparator &comparator, HashFunction<KeyType> hash_fn)
    : buffer_pool_manager_(buffer_pool_manager), comparator_(comparator), hash_fn_(std::move(hash_fn)) {
  //  implement me!
  Page* hash_table_directory_page = buffer_pool_manager_->NewPage(&directory_page_id_);
  HashTableDirectoryPage* page_data = reinterpret_cast<HashTableDirectoryPage*>(hash_table_directory_page->GetData());
  // initialize with 1 bit, 1 buckets
  page_id_t bucket_pid;
  buffer_pool_manager_->NewPage(&bucket_pid);
  page_data->SetBucketPageId(static_cast<uint32_t>(0), bucket_pid);

  
  page_data->SetLocalDepth(0, 0);
  // page_data->PrintDirectory();
  page_data->SetPageId(directory_page_id_);
  // page_data->PrintDirectory();
  buffer_pool_manager_->UnpinPage(directory_page_id_, true);
  buffer_pool_manager_->UnpinPage(bucket_pid, false);

}

/*****************************************************************************
 * HELPERS
 *****************************************************************************/
/**
 * Hash - simple helper to downcast MurmurHash's 64-bit hash to 32-bit
 * for extendible hashing.
 *
 * @param key the key to hash
 * @return the downcasted 32-bit hash
 */
template <typename KeyType, typename ValueType, typename KeyComparator>
uint32_t HASH_TABLE_TYPE::Hash(KeyType key) {
  return static_cast<uint32_t>(hash_fn_.GetHash(key));
}

template <typename KeyType, typename ValueType, typename KeyComparator>
inline uint32_t HASH_TABLE_TYPE::KeyToDirectoryIndex(KeyType key, HashTableDirectoryPage *dir_page) {
  return Hash(key) & dir_page->GetGlobalDepthMask();
}

template <typename KeyType, typename ValueType, typename KeyComparator>
inline uint32_t HASH_TABLE_TYPE::KeyToPageId(KeyType key, HashTableDirectoryPage *dir_page) {
  return dir_page->GetBucketPageId(KeyToDirectoryIndex(key, dir_page));
}

template <typename KeyType, typename ValueType, typename KeyComparator>
HashTableDirectoryPage *HASH_TABLE_TYPE::FetchDirectoryPage() {
  HashTableDirectoryPage* page_data = reinterpret_cast<HashTableDirectoryPage*>(buffer_pool_manager_->FetchPage(directory_page_id_)->GetData());
  return page_data;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
HASH_TABLE_BUCKET_TYPE *HASH_TABLE_TYPE::FetchBucketPage(page_id_t bucket_page_id) {
  HASH_TABLE_BUCKET_TYPE* page_data = reinterpret_cast<HASH_TABLE_BUCKET_TYPE*>(buffer_pool_manager_->FetchPage(bucket_page_id)->GetData());
  return page_data;
}

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::GetValue(Transaction *transaction, const KeyType &key, std::vector<ValueType> *result) {
  table_latch_.RLock();
  
  HashTableDirectoryPage* directory_page = FetchDirectoryPage();
  page_id_t bucket_pid = KeyToPageId(key, directory_page);
  Page* bucket_page = buffer_pool_manager_->FetchPage(bucket_pid);
  HASH_TABLE_BUCKET_TYPE* page_data = reinterpret_cast<HASH_TABLE_BUCKET_TYPE*>(bucket_page->GetData());
  
  bucket_page->RLatch();
  bool isFound = page_data->GetValue(key, comparator_, result);
  buffer_pool_manager_->UnpinPage(directory_page_id_, false);
  buffer_pool_manager_->UnpinPage(bucket_pid, isFound);
  bucket_page->RUnlatch();

  table_latch_.RUnlock();
  return isFound;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::Insert(Transaction *transaction, const KeyType &key, const ValueType &value) {
  table_latch_.RLock();

  HashTableDirectoryPage* directory_page = FetchDirectoryPage();
  page_id_t bucket_pid = KeyToPageId(key, directory_page);
  Page* bucket_page = buffer_pool_manager_->FetchPage(bucket_pid);
  HASH_TABLE_BUCKET_TYPE* page_data = reinterpret_cast<HASH_TABLE_BUCKET_TYPE*>(bucket_page->GetData());
  bucket_page->WLatch();
  
  bool isInserted = page_data->Insert(key, value, comparator_);
  if((!isInserted) && page_data->IsFull()){
    buffer_pool_manager_->UnpinPage(directory_page_id_, false);
    buffer_pool_manager_->UnpinPage(bucket_pid, false);
    bucket_page->WUnlatch();
    table_latch_.RUnlock();
    return SplitInsert(transaction, key, value);
  }
  buffer_pool_manager_->UnpinPage(directory_page_id_, false);
  buffer_pool_manager_->UnpinPage(bucket_pid, isInserted);
  bucket_page->WUnlatch();

  table_latch_.RUnlock();
  return isInserted;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::SplitInsert(Transaction *transaction, const KeyType &key, const ValueType &value) {
  table_latch_.WLock();
  // std::cout<<"splited\n";

  HashTableDirectoryPage* directory_page = FetchDirectoryPage();
  page_id_t bucket_pid = KeyToPageId(key, directory_page);
  Page* bucket_page = buffer_pool_manager_->FetchPage(bucket_pid);
  HASH_TABLE_BUCKET_TYPE* page_data = reinterpret_cast<HASH_TABLE_BUCKET_TYPE*>(bucket_page->GetData());

  bucket_page->WLatch();

  if(!page_data->IsFull()){
    bool isInserted = page_data->Insert(key, value, comparator_);
    
    buffer_pool_manager_->UnpinPage(directory_page_id_, false);
    buffer_pool_manager_->UnpinPage(bucket_pid, isInserted);
    bucket_page->WUnlatch();
    table_latch_.WUnlock();

    return isInserted;
  }

  uint32_t bucket_index = KeyToDirectoryIndex(key, directory_page);
  
  if(directory_page->GetGlobalDepth() == directory_page->GetLocalDepth(bucket_index)){
    directory_page->IncrGlobalDepth();
  }
  directory_page->IncrLocalDepth(bucket_index);
  uint32_t new_bucket_index = directory_page->GetSplitImageIndex(bucket_index);
  
  page_id_t new_bucket_pid;
  HASH_TABLE_BUCKET_TYPE* new_page_data = reinterpret_cast<HASH_TABLE_BUCKET_TYPE*>(buffer_pool_manager_->NewPage(&new_bucket_pid)->GetData());
  
  // uint32_t num_need_change_pointer = 1 << (directory_page->GetGlobalDepth() - directory_page->GetLocalDepth(bucket_index) + 1);
  // uint32_t num_changed = 2;
  
  directory_page->SetLocalDepth(new_bucket_index, directory_page->GetLocalDepth(bucket_index));
  directory_page->SetBucketPageId(new_bucket_index, new_bucket_pid);

  for(uint32_t i = 0; i < directory_page->Size(); i++){
    if(directory_page->GetBucketPageId(i) == bucket_pid){
      if((i >> (directory_page->GetLocalDepth(bucket_index) - 1)) == (bucket_index >> (directory_page->GetLocalDepth(bucket_index) - 1))){
        directory_page->SetLocalDepth(i, directory_page->GetLocalDepth(bucket_index));
        directory_page->SetBucketPageId(i, bucket_pid);
      }else{
        directory_page->SetLocalDepth(i, directory_page->GetLocalDepth(new_bucket_index));
        directory_page->SetBucketPageId(i, new_bucket_pid);
      }
    }
  }

  //rehash
  
  bool isRehashed = false;
  for(uint32_t i = 0; i < BUCKET_ARRAY_SIZE; i++){
    KeyType temp_key = page_data->KeyAt(i);
    // uint32_t new_index = Hash(temp_key) & directory_page->GetLocalDepthMask(bucket_index);
    // uint32_t temp_bucket_index = new_bucket_index & directory_page->GetLocalDepthMask(new_bucket_index);
    if(static_cast<page_id_t>(KeyToPageId(temp_key, directory_page)) != bucket_pid){
      isRehashed = true;
      ValueType temp_value = page_data->ValueAt(i);
      new_page_data->Insert(temp_key, temp_value, comparator_);
      page_data->RemoveAt(i);
    }
  }
  bucket_page->WUnlatch();
  buffer_pool_manager_->UnpinPage(bucket_pid, isRehashed);
  buffer_pool_manager_->UnpinPage(new_bucket_pid, true);
  
  buffer_pool_manager_->UnpinPage(directory_page_id_, true);
  // std::cout<<"splited\n";
  // directory_page->PrintDirectory();

  table_latch_.WUnlock();
  bool isInserted = Insert(transaction, key, value);
  return isInserted;
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::Remove(Transaction *transaction, const KeyType &key, const ValueType &value) {
  table_latch_.RLock();
  HashTableDirectoryPage* directory_page = FetchDirectoryPage();
  page_id_t bucket_pid = KeyToPageId(key, directory_page);
  Page* bucket_page = buffer_pool_manager_->FetchPage(bucket_pid);
  HASH_TABLE_BUCKET_TYPE* page_data = reinterpret_cast<HASH_TABLE_BUCKET_TYPE*>(bucket_page->GetData());

  bucket_page->WLatch();
  bool isRemoved = page_data->Remove(key, value, comparator_);
  if(isRemoved && page_data->IsEmpty()){
    buffer_pool_manager_->UnpinPage(directory_page_id_, false);
    buffer_pool_manager_->UnpinPage(bucket_pid, true);
    bucket_page->WUnlatch();
    table_latch_.RUnlock();
    Merge(transaction, key, value);
    return true;
  }

  buffer_pool_manager_->UnpinPage(directory_page_id_, false);
  buffer_pool_manager_->UnpinPage(bucket_pid, isRemoved);
  bucket_page->WUnlatch();

  table_latch_.RUnlock();
  return isRemoved;
}

/*****************************************************************************
 * MERGE
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_TYPE::Merge(Transaction *transaction, const KeyType &key, const ValueType &value) {
  table_latch_.WLock();
  
  HashTableDirectoryPage* directory_page = FetchDirectoryPage();
  page_id_t bucket_pid = KeyToPageId(key, directory_page);
  Page* bucket_page = buffer_pool_manager_->FetchPage(bucket_pid);
  HASH_TABLE_BUCKET_TYPE* page_data = reinterpret_cast<HASH_TABLE_BUCKET_TYPE*>(bucket_page->GetData());
  uint32_t bucket_index = KeyToDirectoryIndex(key, directory_page);
  bool isUnlocked = false;

  bucket_page->RLatch();

  if((!page_data->IsEmpty()) || (directory_page->GetGlobalDepth() == 0)){
    bucket_page->RUnlatch();
    buffer_pool_manager_->UnpinPage(bucket_pid, false);
    buffer_pool_manager_->UnpinPage(directory_page_id_, true);
  
    table_latch_.WUnlock();
    return;
  }

  if(page_data->IsEmpty() && (directory_page->GetLocalDepth(bucket_index) > 0)){

    uint32_t new_bucket_index = directory_page->GetSplitImageIndex(bucket_index);
    // std::cout<<directory_page->GetGlobalDepth()<<std::endl;
    // std::cout<<bucket_index<<" "<<new_bucket_index<<std::endl;
    // std::cout<<directory_page->GetLocalDepth(bucket_index)<<" "<<directory_page->GetLocalDepth(new_bucket_index)<<" \n";
    // std::cout<<"merged\n";
    if((directory_page->GetLocalDepth(bucket_index) == directory_page->GetLocalDepth(new_bucket_index))
      && (directory_page->GetBucketPageId(bucket_index) != directory_page->GetBucketPageId(new_bucket_index))){
      
      directory_page->DecrLocalDepth(bucket_index);
      directory_page->DecrLocalDepth(new_bucket_index);
      
      
      for(uint32_t i = 0; i < directory_page->Size(); i++){
        if((directory_page->GetBucketPageId(i) == directory_page->GetBucketPageId(new_bucket_index)) 
          && (i != new_bucket_index)){
          directory_page->DecrLocalDepth(i);
        }
        if(directory_page->GetBucketPageId(i) == bucket_pid){
          directory_page->SetLocalDepth(i, directory_page->GetLocalDepth(new_bucket_index));
          directory_page->SetBucketPageId(i, directory_page->GetBucketPageId(new_bucket_index));
        }
      }
      // directory_page->SetBucketPageId(bucket_index, directory_page->GetBucketPageId(new_bucket_index));

      // buffer_pool_manager_->UnpinPage(bucket_pid, false);
      // buffer_pool_manager_->DeletePage(bucket_pid);
      if(directory_page->CanShrink()){
        directory_page->DecrGlobalDepth();
        // std::cout<<"shrink\n";
      }
      bucket_page->RUnlatch();
      buffer_pool_manager_->UnpinPage(bucket_pid, false);
      buffer_pool_manager_->UnpinPage(directory_page_id_, true);
      table_latch_.WUnlock();
      isUnlocked = true;
      
      Merge(transaction, key, value);
    }
    
  }

  if(!isUnlocked){
    bucket_page->RUnlatch();
    buffer_pool_manager_->UnpinPage(bucket_pid, false);
    buffer_pool_manager_->UnpinPage(directory_page_id_, true);
    table_latch_.WUnlock();
  }

  
  // std::cout<<"Merged\n";
  // directory_page->PrintDirectory();
  // bucket_page->RUnlatch();

  
}

/*****************************************************************************
 * GETGLOBALDEPTH - DO NOT TOUCH
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
uint32_t HASH_TABLE_TYPE::GetGlobalDepth() {
  table_latch_.RLock();
  HashTableDirectoryPage *dir_page = FetchDirectoryPage();
  uint32_t global_depth = dir_page->GetGlobalDepth();
  assert(buffer_pool_manager_->UnpinPage(directory_page_id_, false, nullptr));
  table_latch_.RUnlock();
  return global_depth;
}

/*****************************************************************************
 * VERIFY INTEGRITY - DO NOT TOUCH
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_TYPE::VerifyIntegrity() {
  table_latch_.RLock();
  HashTableDirectoryPage *dir_page = FetchDirectoryPage();
  dir_page->VerifyIntegrity();
  assert(buffer_pool_manager_->UnpinPage(directory_page_id_, false, nullptr));
  table_latch_.RUnlock();
}

/*****************************************************************************
 * TEMPLATE DEFINITIONS - DO NOT TOUCH
 *****************************************************************************/
template class ExtendibleHashTable<int, int, IntComparator>;

template class ExtendibleHashTable<GenericKey<4>, RID, GenericComparator<4>>;
template class ExtendibleHashTable<GenericKey<8>, RID, GenericComparator<8>>;
template class ExtendibleHashTable<GenericKey<16>, RID, GenericComparator<16>>;
template class ExtendibleHashTable<GenericKey<32>, RID, GenericComparator<32>>;
template class ExtendibleHashTable<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
