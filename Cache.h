//
// Created by X G and HP on 2/17/22.
//

#ifndef HTTP_PROXY_SERVER_CACHE_H
#define HTTP_PROXY_SERVER_CACHE_H

#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

// Def the double linked node
struct ListNode{
    std::string key; // Define the key for the url
    std::vector<char> response; // Define the HTTP Response
    ListNode * prev;
    ListNode * next;
    ListNode():key(""), response({}),prev(nullptr), next(nullptr){} // default constructor
    ListNode(string key, vector<char> response): key(key), response(response), prev(nullptr), next(nullptr){}

};

/**
 * Define the LRU Cache
 */
class Cache {
private:
    unordered_map<string,ListNode*> cache;
    ListNode* head;
    ListNode* tail;
    int size;
    int capacity;

protected:

    /**
     * Move the curr node to the head
     * @param node to be move to the head
     */
    void move2Head(ListNode* node);

    /**
     * remove the tail of ll and return the removed node
     * @return the node to be removed
     */
    ListNode *rmTail();

    /**
     * Add the node into the head of ll
     * @param pNode to be added
     */
    void add2Head(ListNode *pNode);


public:
    /**
     * The constructor of the LRU cache with size 0
     * @param my_capacity of cache{
     */
    Cache(int my_capacity):capacity(my_capacity),size(0){
        head = new ListNode();
        tail=new ListNode();
        head->next=tail;
        tail->prev=head;
    }

    /**
     * Get the key
     * @param key of the url
     * @return {} when not found, or return response
     */
    std::vector<char> get(string key);



/**
     * Put the key into the Cache
     * @param key url
     * @param response
     */
    void put(string key,vector<char> response);


};


#endif //HTTP_PROXY_SERVER_CACHE_H
