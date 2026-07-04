#pragma once 

#include <list>
#include <unordered_map>


template<typename KEY, typename VAL>
class LRUcache{
    private:
        struct Node{
            KEY key;
            VAL val;
            int ref;
            
            std::list<Node>::iterator list_pos;
            Node(const KEY& k, VAL&& v)
                :key(k), val(std::move(v)), ref(1) {}
            Node(const KEY& k, const VAL& v)
                :key(k), val(v), ref(1) {}
        };

    public:
        using ListNodeIterator=std::list<Node>::iterator;
        class HandleCache{
            public:
                ~HandleCache(){
                    
                }
            private:
                HandleCache():cache(nullptr), node(nullptr) {}
                HandleCache(LRUcache* cache, Node* node): cache(cache), node(node) {

                }
                LRUcache* cache;
                Node* node;

        };

    private:
        void ref_node(Node* node){
            if(node->ref==1){
                //not_use->in_use
                inuse.splice()
            }
            node->ref++;
        }

        void uref_node(Node* node){

        }
        std::list<Node> in_use; //当前使用节点列表
        std::list<Node> not_use; //未使用节点列表；
                                 //相当于将一个列表拆成两部分，只能从not_use当中淘汰，in_use一定是最近使用的

        std::unordered_map<KEY,ListNodeIterator> cache;
};