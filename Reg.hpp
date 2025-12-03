#pragma once

#include <unordered_map>
#include <vector>
#include <bitset>
#include <tuple>

#include "Comps.hpp"

using Entity = uint32_t;

class Registry {
protected:
    static const size_t maxComp = std::tuple_size_v<AllComponents>;
    std::unordered_map<Entity, std::bitset<maxComp>> entToBit; //mapping entities to what comps they have
    std::vector<Entity> removedEnt; //cached free id spots for createentity()
    std::vector<Entity> toRemove; //to prevent errors with altering container size while looping through it
    std::unordered_map<Entity, std::bitset<maxComp>> toAdd; //^same logic as above 

    template<typename C>
    struct ComponentStorage {
        std::vector<C> data;
        std::unordered_map<Entity, size_t> entityToIndex;
        std::vector<Entity> indexToEntity; //for comp removal
    };

    //sourced from https://stackoverflow.com/questions/18063451/get-index-of-a-tuple-elements-type
    template <class T, class Tuple>
    struct Index;

    template <class T, class... Types>
    struct Index<T, std::tuple<T, Types...>> {
        static const std::size_t value = 0;
    };


    template <class T, class U, class... Types>
    struct Index<T, std::tuple<U, Types...>> {
        static const std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
    };
    //end source

    template<typename C>
    ComponentStorage<C>& storage() {
        // One instance per component type C
        static ComponentStorage<C> s;
        return s;
    }

    template<std::size_t... I>
    void destroyComps(Entity e, std::index_sequence<I...>)
    {
        // Fold expression: tries all I, but only calls remove for the matching ones
        ((entToBit[e].test(I)
            ? remove<std::tuple_element_t<I, AllComponents>>(e)
            : void()), ...);
    }

    void HandleCreationAndDestruction() //this is to prevent adding or deleting entities mid loop
    {
        //addition
        for (auto e : toAdd)
        {
            entToBit.insert(e);
        }
        toAdd.clear();

        //destruction
        for (auto e : toRemove)
        {
            destroyComps(e, std::make_index_sequence<std::tuple_size_v<AllComponents>>{});
            entToBit.erase(entToBit.find(e));
            removedEnt.push_back(e);
        }
        toRemove.clear();
    }
    
public:
    Entity CreateEntity()
    {
        //returns entity and adds the bitset entry to toadd vector
        Entity index;
        if (removedEnt.size() == 0)
        {
            index = entToBit.size() + toAdd.size();
        }
        else
        {
            index = removedEnt.back();
            removedEnt.pop_back();
        }
        toAdd.insert({index, std::bitset<maxComp>{}});
        return index;
    }

    template<typename C>
    void add(Entity e, C component) 
    {
        auto& store = storage<C>();
        store.entityToIndex[e] = store.data.size(); //match index to array with entity
        store.indexToEntity.push_back(e);           //for removal
        store.data.push_back(component);            //add data to array

        //update bitset
        if (toAdd.find(e) != toAdd.end()) //if not been added yet, alter to add
        {
            toAdd[e].set(Index<C,AllComponents>::value,true);
            return;
        }
        entToBit[e].set(Index<C,AllComponents>::value,true);
    }

    template<typename C>
    C* get(Entity e) {
        if (!entToBit.contains(e)) {return nullptr;} //dont allow access to entities that havent been created yet
        auto& store = storage<C>();
        auto it = store.entityToIndex.find(e);
        if (it == store.entityToIndex.end()) return nullptr;
        return &store.data[it->second];
    }

    template<typename C>
    std::vector<Entity> getAllEnt()
    {
        auto& store = storage<C>();
        std::vector<Entity> actualList; //prevent returning not yet created entities
        for (auto ent : store.indexToEntity)
        {
            if (!entToBit.contains(ent)) { continue; }
            actualList.push_back(ent);
        }
        return actualList;
    }

    template<typename... C>
    bool has(Entity e) 
    {
        if (!entToBit.contains(e)) { return false; }
        return (entToBit[e].test(Index<C, AllComponents>::value) && ...);
    }

    template<typename C>
    void remove(Entity e) 
    {
        //check if the array for that component contains an entry for given entity
        auto& store = storage<C>();
        auto it = store.entityToIndex.find(e);
        if (it == store.entityToIndex.end()) return;

        //store the index in the actual array and the index to the last element
        size_t index = it->second;
        size_t lastIndex = store.data.size() - 1;

        //swap the last element with the removed element and update the books 
        store.data[index] = store.data[lastIndex];
        Entity movedEnt = store.indexToEntity[lastIndex];
        store.entityToIndex[movedEnt] = index;
        store.indexToEntity[index] = movedEnt;

        //remove elements
        store.data.pop_back();
        store.indexToEntity.pop_back();
        store.entityToIndex.erase(e);

        //update bitset
        entToBit[e].set(Index<C,AllComponents>::value,false);
    }

    void Destroy(Entity e)
    {
        toRemove.push_back(e);
    }
};