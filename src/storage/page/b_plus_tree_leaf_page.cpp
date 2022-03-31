//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/page/b_plus_tree_leaf_page.cpp
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <sstream>

#include "common/exception.h"
#include "common/rid.h"
#include "storage/page/b_plus_tree_leaf_page.h"

namespace bustub {

/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/

/**
 * Init method after creating a new leaf page
 * Including set page type, set current size to zero, set page id/parent id, set
 * next page id and set max size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::Init(page_id_t page_id, page_id_t parent_id, int max_size) {
  SetPageId(page_id);
  SetParentPageId(parent_id);
  SetMaxSize(max_size);
  SetSize(0);
  SetPageType(IndexPageType::LEAF_PAGE);
  SetNextPageId(INVALID_PAGE_ID);
}

/**
 * Helper methods to set/get next page id
 */
INDEX_TEMPLATE_ARGUMENTS
page_id_t B_PLUS_TREE_LEAF_PAGE_TYPE::GetNextPageId() const { return next_page_id_; }

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::SetNextPageId(page_id_t next_page_id) { next_page_id_ = next_page_id; }

/**
 * Helper method to find the first index i so that array[i].first >= key
 * NOTE: This method is only used when generating index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_LEAF_PAGE_TYPE::KeyIndex(const KeyType &key, const KeyComparator &comparator) const {
  int index = -1;
  if(comparator(array_[GetSize() - 1].first, key) < 0){
    index = GetSize();
    return index;
  }
  // std::cout<<"key: "<<key<<"\n";
  for(int i = 0; i < GetSize(); i++){
    // std::cout<<array_[i].first<<"\n";
    if(comparator(array_[i].first, key) >= 0){
      index = i;
      break;
    }
  }
  return index; 
  // int low = 0;
  // int high = GetSize() - 1;
  // int index = -1;
  // if(high == -1){
  //   return -1;
  // }
  // if(high == 0){
  //   if(comparator(key, array_[0].first) <= 0){
  //     return 0;
  //   }else{
  //     return 1;
  //   }
  // }
  // while(low < high){
  //   if(comparator(key, array_[low].first) <= 0){
  //     index = low;
  //     break;
  //   }
  //   if((comparator(key, array_[high].first) <= 0) && (comparator(key, array_[high - 1].first) > 0)){
  //     index = high;
  //     break;
  //   }
  //   if(comparator(key, array_[high].first) > 0){
  //     index = high + 1;
  //     break;
  //   }
  //   int middle = (low + high)/2;
  //   if(middle > 0){
  //     if((comparator(key, array_[middle].first) <= 0) && (comparator(key, array_[middle - 1].first) > 0)){
  //       index = middle;
  //       break;
  //     }
  //   }
  //   if(comparator(key, array_[middle].first) < 0){
  //     high = middle;
  //   } 
  //   if(comparator(key, array_[middle].first) > 0){
  //     low = middle;
  //   }
  // }
  // return index; 
}

/*
 * Helper method to find and return the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
KeyType B_PLUS_TREE_LEAF_PAGE_TYPE::KeyAt(int index) const {
  // replace with your own code

  return array_[index].first;
}

/*
 * Helper method to find and return the key & value pair associated with input
 * "index"(a.k.a array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
const MappingType &B_PLUS_TREE_LEAF_PAGE_TYPE::GetItem(int index) {
  // replace with your own code
  return array_[index];
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert key & value pair into leaf page ordered by key
 * @return  page size after insertion
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_LEAF_PAGE_TYPE::Insert(const KeyType &key, const ValueType &value, const KeyComparator &comparator) {
  int index = KeyIndex(key, comparator);
  // std::cout<<"Start insert\nindex: "<<index<<" key: "<<key<<"\n";
  if(index == -1){
    //no key
    array_[0].first = key;
    array_[0].second = value;
    IncreaseSize(1);
    return 1;
  }
  if(comparator(key, KeyAt(index)) == 0){
    return GetSize();
  }
  for(int i = GetSize(); i > index; i--){
    array_[i] = array_[i-1];
  }
  array_[index].first = key;
  array_[index].second = value;
  IncreaseSize(1);
  // std::cout<<"Inserted\nkey: "<<array_[index].first<<"\n";
  return GetSize();
}

/*****************************************************************************
 * SPLIT
 *****************************************************************************/
/*
 * Remove half of key & value pairs from this page to "recipient" page
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveHalfTo(BPlusTreeLeafPage *recipient) {
  int size = this->GetSize();
  int index = size - size/2;
  // std::cout<<"MoveHalfTo: size: "<<size<<" index: "<<index<<"\n";
  // std::cout<<"array key: "<<array_[index].first<<" "<<array_[index+1].first<<"\n";
  for(int i = index; i < size; i++){
    recipient->CopyLastFrom(array_[i]);
  }
  this->DecreaseSize(size/2);
}

/*
 * Copy starting from items, and copy {size} number of elements into me.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::CopyNFrom(MappingType *items, int size) {}

/*****************************************************************************
 * LOOKUP
 *****************************************************************************/
/*
 * For the given key, check to see whether it exists in the leaf page. If it
 * does, then store its corresponding value in input "value" and return true.
 * If the key does not exist, then return false
 */
INDEX_TEMPLATE_ARGUMENTS
bool B_PLUS_TREE_LEAF_PAGE_TYPE::Lookup(const KeyType &key, ValueType *value, const KeyComparator &comparator) const {
  int index = KeyIndex(key, comparator);
  if((index == -1) || (index >= GetSize())){
    return false;
  }
  if(comparator(key, array_[index].first) == 0){
    *value = array_[index].second;
    return true;
  }
  return false;
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * First look through leaf page to see whether delete key exist or not. If
 * exist, perform deletion, otherwise return immediately.
 * NOTE: store key&value pair continuously after deletion
 * @return   page size after deletion
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_LEAF_PAGE_TYPE::RemoveAndDeleteRecord(const KeyType &key, const KeyComparator &comparator) { 
  int index = KeyIndex(key, comparator);
  if((index == -1) || (index > GetSize())){
    return GetSize();
  }
  if(comparator(key, array_[index].first) == 0){
    for(int i = index; i < GetSize() - 1; i++){
      array_[i] = array_[i+1];
    }
    DecreaseSize(1);
  }
  return GetSize(); 
}

/*****************************************************************************
 * MERGE
 *****************************************************************************/
/*
 * Remove all of key & value pairs from this page to "recipient" page. Don't forget
 * to update the next_page id in the sibling page
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveAllTo(BPlusTreeLeafPage *recipient) {
  for(int i = 0; i < GetSize(); i++){
    recipient->CopyLastFrom(array_[i]);
  }
  recipient->SetNextPageId(GetNextPageId());
  this->SetSize(0);
}

/*****************************************************************************
 * REDISTRIBUTE
 *****************************************************************************/
/*
 * Remove the first key & value pair from this page to "recipient" page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveFirstToEndOf(BPlusTreeLeafPage *recipient) {
  recipient->CopyLastFrom(array_[0]);
  for(int i = 0; i < this->GetSize() - 1; i++){
    array_[i] = array_[i+1];
  }
  this->DecreaseSize(1);
}

/*
 * Copy the item into the end of my item list. (Append item to my array)
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::CopyLastFrom(const MappingType &item) {
  array_[GetSize()] = item;
  IncreaseSize(1);
}

/*
 * Remove the last key & value pair from this page to "recipient" page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveLastToFrontOf(BPlusTreeLeafPage *recipient) {
  recipient->CopyFirstFrom(array_[GetSize() - 1]);
  this->DecreaseSize(1);
}

/*
 * Insert item at the front of my items. Move items accordingly.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::CopyFirstFrom(const MappingType &item) {
  for(int i = GetSize(); i > 0; i--){
    array_[i] = array_[i-1];
  }
  array_[0] = item;
}

template class BPlusTreeLeafPage<GenericKey<4>, RID, GenericComparator<4>>;
template class BPlusTreeLeafPage<GenericKey<8>, RID, GenericComparator<8>>;
template class BPlusTreeLeafPage<GenericKey<16>, RID, GenericComparator<16>>;
template class BPlusTreeLeafPage<GenericKey<32>, RID, GenericComparator<32>>;
template class BPlusTreeLeafPage<GenericKey<64>, RID, GenericComparator<64>>;
}  // namespace bustub
