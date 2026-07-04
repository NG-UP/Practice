#pragma once 

#include <list>
#include <unordered_map>
#include <cassert>

template<typename T>
struct DefaultValueDeleter{
    void operator() (T& value){
        if constexpr (std::is_pointer_v<T>){
            delete value;
            value=nullptr;
        }
    }
};

template<typename KEY, typename VAL, class ValueDeleter=DefaultValueDeleter<VAL>>
class LRUcache{
    private:
        struct Node{
            KEY key;
            VAL val;
            int ref;
            
            typename std::list<Node>::iterator list_pos;
            Node(const KEY& k, VAL&& v)
                :key(k), val(std::move(v)), ref(1) {}
            Node(const KEY& k, const VAL& v)
                :key(k), val(v), ref(1) {}
        };

    public:
        LRUcache(size_t capacity): capacity(capacity), size(0) {}

        ~LRUcache(){
            assert(in_use.empty());
            for(auto it=not_use.begin();it!=not_use.end();){
                Node& node=*it;
                cache.erase(node.key);

                value_deleter(node.val);
                
                it=not_use.erase(it);
                size--;
            }
        }

        LRUcache(const LRUcache& )=delete;
        LRUcache& operator = (const LRUcache&)=delete;
        LRUcache& operator = (LRUcache&&) noexcept=delete;
        LRUcache(LRUcache&& other) noexcept=delete; 

        using ListNodeIterator= typename std::list<Node>::iterator;
        class HandleCache{
            public:
                ~HandleCache(){
                    reset();
                }

                HandleCache(const HandleCache& )=delete;
                HandleCache& operator = (const HandleCache&)=delete;
                HandleCache& operator = (HandleCache&&) noexcept=delete;
                HandleCache(HandleCache&& other) noexcept{
                    cache=other.cache;
                    node=other.node;
                    other.cache=nullptr;
                    other.node=nullptr;
                }

                void reset(){
                    if(valid()){
                        cache->uref_node(node);
                        cache=nullptr;
                        node=nullptr;
                    }
                }

                VAL& value(){
                    assert(valid());
                    return node->val;
                }

                bool valid() const {return cache && node;}
                explicit operator bool() const {return valid();}
            private:
                friend class LRUcache<KEY,VAL>;
                HandleCache():cache(nullptr), node(nullptr) {}
                explicit HandleCache(LRUcache* cache, Node* node): cache(cache), node(node) {
                    if(cache && node){
                        cache->ref_node(node);
                    }
                }
                LRUcache* cache;
                Node* node;
        };

        HandleCache get(KEY& key){
            auto it=cache.find(key);
            if(it!=cache.end()){
                return HandleCache(this, &(*it->second));
            }
            return HandleCache();
        }
        template<typename V>
        HandleCache put(const KEY& key, V&& val){
            static_assert(std::is_constructible_v<VAL,V&&>,"V 是用来构造 val的");
            auto it=cache.find(key);
            if(it!=cache.end()){
                auto node_iter=it.second;
                node_iter->value=std::forward<V>(val);
                return HandleCache(this, &(*node_iter));
            }
            auto node_iter=not_use.emplace(not_use.end(), key, std::forward<V>(val));
            node_iter->list_pos=node_iter;
            size++;
            evict_if_needed();
            return HandleCache(this, &(*node_iter));
        }

    private:
        void ref_node(Node* node){
            if(node->ref==1){
                //not_use->in_use
                in_use.splice(in_use.end(),not_use,node->list_pos);
            }
            node->ref++;
        }

        void uref_node(Node* node){
            assert(node->ref >1);
            node->ref--;
            if(node->ref==1){
                //in_use->not_use
                not_use.splice(not_use.end(),in_use,node->list_pos);
            }
        }

        void evict_if_needed(){
            if(capacity>0){
                while(size>capacity && !not_use.empty()){
                    auto node_iter=not_use.begin();
                    Node& node=*node_iter;
                    cache.erase(node.key);

                    value_deleter(node.value);
                    not_use.erase(node_iter);
                    size--;
                }
            }
        }

        size_t capacity;
        size_t size;
        std::list<Node> in_use; //当前使用节点列表
        std::list<Node> not_use; //未使用节点列表；
                                 //相当于将一个列表拆成两部分，只能从not_use当中淘汰，in_use一定是最近使用的

        std::unordered_map<KEY,ListNodeIterator> cache;

        ValueDeleter value_deleter;
};