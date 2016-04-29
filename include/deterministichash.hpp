// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2010 Guy Blelloch and the PBBS team
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef _PBBS_PCTL_DETERMINISTIC_HASH_H_
#define _PBBS_PCTL_DETERMINISTIC_HASH_H_

#include "utils.hpp"
#include "prandgen.hpp"
#include "dpsdatapar.hpp"

namespace pasl {
namespace pctl {
  
using intT = int;
using uintT = unsigned int;
    
using namespace std;

// A "history independent" hash table that supports insertion, and searching
// It is described in the paper
//   Guy E. Blelloch, Daniel Golovin
//   Strongly History-Independent Hashing with Applications
//   FOCS 2007: 272-282
// At any quiescent point (when no operations are actively updating the
//   structure) the state will depend only on the keys it contains and not
//   on the history of the insertion order.
// Insertions can happen in parallel, but they cannot overlap with searches
// Searches can happen in parallel
// Deletions must happen sequentially
template <class HASH, class intT>
class Table {
private:
  typedef typename HASH::eType eType;
  typedef typename HASH::kType kType;
  intT m;
  intT mask;
  eType empty;
  HASH hash_struct;
  eType* table;
  intT* compactL;
  
  // needs to be in separate routine due to Cilk bugs
  static void clear(eType* A, intT n, eType v) {
    pmem::fill(A, A + n, v);
  }
  
  struct notEmptyF {
    eType e; notEmptyF(eType _e) : e(_e) {}
    int operator() (eType a) {return e != a;}};
  
  uintT hash_to_range(intT h) {
    return h & mask;
  }
  intT first_index(kType v) {
    return hash_to_range(hash_struct.hash(v));
  }
  intT increment_index(intT h) {
    return hash_to_range(h + 1);
  }
  intT decrement_index(intT h) {
    return hash_to_range(h - 1);
  }
  bool less_index(intT a, intT b) {
    return 2 * hash_to_range(a - b) > m;
  }
  
  
public:
  // Size is the maximum number of values the hash table will hold.
  // Overfilling the table could put it into an infinite loop.
  Table(intT size, HASH hash) : m(1 << utils::log2Up(100 + 2 * size)),
                                mask(m - 1),
                                empty(hash.empty()),
                                hash_struct(hash),
                                table(newA(eType, m)),
                                compactL(NULL) {
    clear(table, m, empty);
  }
  
  // Deletes the allocated arrays
  void del() {
    free(table);
    if (compactL != NULL) {
      free(compactL);
    }
  }
  
  // prioritized linear probing
  //   a new key will bump an existing key up if it has a higher priority
  //   an equal key will replace an old key if replaceQ(new,old) is true
  // returns 0 if not inserted (i.e. equal and replaceQ false) and 1 otherwise
  bool insert(eType v) {
    kType vkey = hash_struct.get_key(v);
    intT h = first_index(vkey);
    while (true) {
      eType c;
      int cmp;
      bool swapped = 0;
      c = table[h];
      cmp = (c == empty) ? 1 : hash_struct.cmp(vkey, hash_struct.get_key(c));
      
      // while v is higher priority than entry at table[h] try to swap it in
      while (cmp > 0 && !(swapped = utils::CAS(&table[h], c, v))) {
        c = table[h];
        cmp = hash_struct.cmp(vkey, hash_struct.get_key(c));
      }
      
      // if swap succeeded either we are done (if swapped with empty)
      // or we have a new lower priority value we have to insert
      if (swapped) {
        if (c == empty) {
          return 1; // done
        } else {
          v = c;
          vkey = hash_struct.get_key(v);
        } // new value to insert
      } else {
        // if swap did not succeed then priority of table[h] >= priority of v
        
        // if equal keys (priorities equal) then either quit or try to replace
        while (cmp == 0) {
          // if other equal element does not need to be replaced then quit
          if (!hash_struct.replaceQ(v, c)) {
            return 0;
          }
          
          // otherwise try to replace (atomically) and quit if successful
          else if (utils::CAS(&table[h], c, v)) {
            return 1;
          }
          // otherwise failed due to concurrent write, try again
          c = table[h];
          cmp = hash_struct.cmp(vkey, hash_struct.get_key(c));
        }
      }
      
      // move to next bucket
      h = increment_index(h);
    }
    return 0; // should never get here
  }
  
  // needs to be more thoroughly tested
  // currently always returns true
  bool remove(kType v) {
    intT i = first_index(v);
    int cmp;
    
    // find first element less than or equal to v in priority order
    intT j = i;
    eType c = table[j];
    while ((cmp = (c == empty) ? 1 : hash_struct.cmp(v, hash_struct.get_key(c))) < 0) {
      j = increment_index(j);
      c = table[j];
    }
    do {
      if (cmp > 0) {
        // value at j is less than v, need to move down one
        if (j == i) {
          return true;
        }
        j = decrement_index(j);
      }
      else { // (cmp == 0)
        // found the element to delete at location j
        
        // Find next available element to fill location j.
        // This is a little tricky since we need to skip over elements for
        // which the hash index is greater than j, and need to account for
        // things being moved around by others as we search.
        // Makes use of the fact that values in a cell can only decrease
        // during a delete phase as elements are moved from the right to left.
        intT jj = increment_index(j);
        eType x = table[jj];
        while (x != empty && less_index(j, first_index(hash_struct.get_key(x)))) {
          jj = increment_index(jj);
          x = table[jj];
        }
        intT jjj = decrement_index(jj);
        while (jjj != j) {
          eType y = table[jjj];
          if (y == empty || !less_index(j, first_index(hash_struct.get_key(y)))) {
            x = y;
          }
          jjj = decrement_index(jjj);
        }
        
        // try to copy the the replacement element into j
        if (utils::CAS(&table[j], c, x)) {
          // swap was successful
          // if the replacement element was empty, we are done
          if (x == empty) {
            return true;
          }
          
          // Otherwise there are now two copies of the replacement element x
          // delete one copy (probably the original) by starting to look at jj.
          // Note that others can come along in the meantime and delete
          // one or both of them, but that is fine.
          v = hash_struct.get_key(x);
          j = jj;
        } else {
          // if fails then c (with value v) has been deleted or moved to a lower
          // location by someone else.
          // start looking at one location lower
          if (j == i) {
            return true;
          }
          j = decrement_index(j);
        }
      }
      c = table[j];
      cmp = (c == empty) ? 1 : hash_struct.cmp(v, hash_struct.get_key(c));
    } while (cmp >= 0);
    return true;
  }
  
  // Returns the value if an equal value is found in the table
  // otherwise returns the "empty" element.
  // due to prioritization, can quit early if v is greater than cell
  eType find(kType v) {
    intT h = first_index(v);
    eType c = table[h];
    while (true) {
      if (c == empty) {
        return empty;
      }
      int cmp = hash_struct.cmp(v, hash_struct.get_key(c));
      if (cmp >= 0) {
        if (cmp > 0) {
          return empty;
        } else {
          return c;
        }
      }
      h = increment_index(h);
      c = table[h];
    }
  }
  
  // returns the number of entries
  intT count() {
    return level1::reduce(table, table + m, (intT)0, [&] (intT x, intT y) {
      return x + y;
    }, [&] (eType a) {
        return empty != a;
    });
//    return sequence::mapReduce<intT>(table,m,utils::addF<intT>(),notEmptyF(empty));
  }
  
  // returns all the current entries compacted into a sequence
  parray<eType> entries() {
    return filter(table, table + m, [&] (eType x) {
      return x != empty;
    });
  }
  
  // prints the current entries along with the index they are stored at
  void print() {
    cout << "vals = ";
    for (intT i = 0; i < m; i++)
      if (table[i] != empty)
        cout << i << ":" << table[i] << ",";
    cout << endl;
  }
};

template <class Hash, class ET, class intT>
parray<ET> remove_duplicates(parray<ET>& a, intT m, Hash hash) {
#ifdef TIME_MEASURE
      auto start = std::chrono::system_clock::now();
#endif
  Table<Hash, intT> t(m, hash);
#ifdef TIME_MEASURE
      auto end = std::chrono::system_clock::now();
      std::chrono::duration<float> diff = end - start;
      printf ("exectime creation %.3lf\n", diff.count());

      start = std::chrono::system_clock::now();
#endif
  parallel_for(a.begin(), a.end(), [&] (ET* it) {
    t.insert(*it);
  });
#ifdef TIME_MEASURE
      end = std::chrono::system_clock::now();
      diff = end - start;
      printf ("exectime insertion %.3lf\n", diff.count());

      start = std::chrono::system_clock::now();
#endif
  parray<ET> r = t.entries();
#ifdef TIME_MEASURE
      end = std::chrono::system_clock::now();
      diff = end - start;
      printf ("exectime entries %.3lf\n", diff.count());
#endif

  t.del();
  return r;
}

template <class HASH, class ET>
parray<ET> remove_duplicates(parray<ET>& a, HASH hashF) {
  return remove_duplicates(a, (intT)a.size(), hashF);
}

template <class intT>
struct hash_int {
  typedef intT eType;
  typedef intT kType;

  eType empty() const {
    return -1;
  }

  kType get_key(eType v) const {
    return v;
  }

  intT hash(kType v) const {
    return prandgen::hashu(v);
  }

  int cmp(kType v, kType b) const {
    return (v > b) ? 1 : ((v == b) ? 0 : -1);
  }

  bool replaceQ(eType v, eType b) const {
    return 0;
  }
};

// works for non-negative integers (uses -1 to mark cell as empty)

static parray<intT> remove_duplicates(parray<intT>& a) {
  return remove_duplicates(a, hash_int<intT>());
}

//typedef Table<hashInt> IntTable;
//static IntTable makeIntTable(int m) {return IntTable(m,hashInt());}
template <class intT>
static Table<hash_int<intT>, intT> make_int_table(intT m) {
  return Table<hash_int<intT>, intT>(m, hash_int<intT>());
}

struct hash_string {
  typedef char* eType;
  typedef char* kType;
  
  eType empty() const {
    return NULL;
  }

  kType get_key(eType v) const {
    return v;
  }
  
  uintT hash(kType s) const {
    uintT hash = 0;
    int len = strlen(s);
    for (int i = 0; i < len; ++i) {
      hash = s[i] + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
  }
  
  int cmp(kType s, kType s2) const {
    return strcmp(s, s2);
  }
  
  bool replaceQ(eType s, eType s2) const {
    return 0;
  }
};

static parray<char*> remove_duplicates(parray<char*>& a) {
  return remove_duplicates(a, hash_string());
}

template <class intT>
static Table<hash_string,intT> make_str_table(intT m) {
  return Table<hash_string,intT>(m, hash_string());
}

template <class KEYHASH, class DTYPE>
struct hash_pair {
  KEYHASH key_hash;
  typedef typename KEYHASH::kType kType;
  typedef pair<kType, DTYPE>* eType;

  eType empty() const {
    return NULL;
  }
  
  hash_pair(KEYHASH _k) : key_hash(_k) {}
  
  kType get_key(eType v) const {
    return v->first;
  }
  
  uintT hash(kType s) const {
    return key_hash.hash(s);
  }

  int cmp(kType s, kType s2) const {
    return key_hash.cmp(s, s2);
  }
  
  bool replaceQ(eType s, eType s2) const {
    return s->second > s2->second;
  }
};

static parray<pair<char*, intT>*> remove_duplicates(parray<pair<char*, intT>*>& a) {
  return remove_duplicates(a, hash_pair<hash_string, intT>(hash_string()));
}
    
template <class ET, class Hash>
parray<ET> run_dict(const parray<ET>& insert, const parray<ET>& remove, const parray<ET>& find, const Hash& hash) {
  Table<Hash, intT> table(insert.size(), hash);
  parallel_for(insert.begin(), insert.end(), [&] (ET* it) {
    table.insert(*it);
  });
/*  for (auto it : insert) {
    table.insert(it);
  }*/
  std::cerr << "Insert: " << table.count() << std::endl;
  parallel_for(remove.begin(), remove.end(), [&] (ET* it) {
    table.remove(hash.get_key(*it));
  });
/*  for (auto it : remove) {
    table.remove(hash.get_key(it));
  }*/
  std::cerr << "Remove: " << table.count() << std::endl;
  parray<ET> result(find.size());
  parallel_for((intT)0, (intT)find.size(), [&] (int i) {
    result[i] = table.find(hash.get_key(find[i]));
  });
/*  for (int i = 0; i < find.size(); i++) {
    result[i] = table.find(hash.get_key(find[i]));
  }*/
  return result;
}

} // end namespace
} // end namespace

#endif // _PBBS_PCTL_DETERMINSTIC_HASH_H_
