//
// Created by X G on 2/17/22.
//

#include "Cache.h"

/**
 * Get the response by key
 * @param key of the url
 * @return {} when not found, or return response
 */
std::vector<char> Cache::get(string key){
    // if not found in the cache(hash map) return empty {}
    if(!cache.count(key)){
        return {};
    }
    // else return response
    ListNode * node = this->cache[key];
    // move the head to the listnode head
    move2Head(node);
    return node->response;
}

/**
 * remove the tail of ll and return the removed node
 * @return the node to be removed
 */
ListNode *Cache::rmTail() {
    ListNode * node=tail->prev;
    node->prev->next=node->next;
    node->next->prev=node->prev;
    return node;
}

/**
 * Add the node into the head of ll
 * @param pNode to be added
 */
void Cache::add2Head(ListNode *pNode) {
    pNode->next = this->head->next;
    pNode->prev = this->head;
    this->head->next->prev = pNode;
    this->head->next = pNode;
}

/**
     * Put the key into the Cache
     * @param key url
     * @param response
     */
void Cache::put(string key,vector<char> response){
    // if the key is not in the cache
    if(!cache.count(key)){
        this->size++; // increase the size
        ListNode * node = new ListNode(key,response);
        this->cache[key] = node; // save the new node into the cache
        // add the new node into the head
        add2Head(node);
        // if the size is beyond the max capacity
        if(this->size > this->capacity){
            // rm the tail of node
            ListNode * node2Rm =  rmTail();
            // rm the key in that hashmap
            this->cache.erase(node2Rm->key);
            // decrease the size
            this->size--;
            // free the node2Rm
            delete(node2Rm);
        }
    }
    else{ // if existed, then update the response and move to the head
        ListNode * toUpdate =  this->cache[key];
        toUpdate->response = response;
        move2Head(toUpdate);
    }
}

/**
 * Move the curr node to the head
 * @param node to be move to the head
 */
void Cache::move2Head(ListNode* node){
    // connect the prev to the next
    node->prev->next = node->next;
    node->next->prev = node->prev;

    // move the node to the head
    node->prev = head;
    node->next = head->next;
    head->next->prev = node;
    head->next = node;
}