//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_replacer.cpp
//
// Identification: src/buffer/lru_replacer.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_replacer.h"
#include <iostream>

namespace bustub {

LRUReplacer::LRUReplacer(size_t num_pages) {}

LRUReplacer::~LRUReplacer(){
  hashmap.clear();
  dll.clear();
  locker.~mutex();
};

bool LRUReplacer::Victim(frame_id_t *frame_id) { 
  locker.lock();
  if(dll.size() != 0){
    *frame_id = dll.back();
    dll.pop_back();
    hashmap.erase(*frame_id);
    locker.unlock();
    return true;
  }else{
    frame_id = nullptr; //fail to assgin nullptr
    locker.unlock();
    return false; 
  }
}

void LRUReplacer::Pin(frame_id_t frame_id) {
  locker.lock();
  if(hashmap.find(frame_id) != hashmap.end()){
    dll.erase(hashmap[frame_id]);
    hashmap.erase(frame_id);
  }
  //do nothing if frame_id does not exist
  locker.unlock();
}

void LRUReplacer::Unpin(frame_id_t frame_id) {
  
  locker.lock();
  if(hashmap.find(frame_id) == hashmap.end()){
    dll.push_front(frame_id);
    hashmap[frame_id] = dll.begin();
  }
  //do nothing if frame_id exists
  locker.unlock();
}

size_t LRUReplacer::Size() { 
  return hashmap.size(); 
}

}  // namespace bustub
