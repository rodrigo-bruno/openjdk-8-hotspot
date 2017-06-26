#ifndef SHARE_VM_LAG1_LAG1OOPCLOSURES_HPP
#define SHARE_VM_LAG1_LAG1OOPCLOSURES_HPP

# include <stdint.h>
# include "oops/oop.hpp"
# include "gc_implementation/g1/g1OopClosures.hpp"


/* TODO: Add here an explanation of all the closures declared in this file */

class LAG1ParScanDSClosure : public G1ParClosureSuper
{
  friend class LAG1ParMarkDSClosure;
  friend class LAG1ParMarkFollowerClosure;
  
 public:
  LAG1ParScanDSClosure(G1CollectedHeap * g1, G1ParScanThreadState * par_scan_state) :
    G1ParClosureSuper(g1, par_scan_state) { }

  template <class T> void do_oop_nv(T * p);
  virtual void do_oop(oop * p)       { do_oop_nv(p); }
  virtual void do_oop(narrowOop * p) { do_oop_nv(p); }
};

class LAG1ParMarkDSClosure : public G1ParClosureSuper
{
  // For offset calculation
  HeapWord * _g1h_reserved_end;
  // Scanner of data-structure fields
  LAG1ParScanDSClosure _ds_scanner;
  
  template <class T> void do_oop_work(T * p);

  /* Encodes the offset of the ptr to the alloc region using the reserved_end of the heap
   * as base */
  uintptr_t calculate_offset(void * region)
    { return pointer_delta(region, _g1h_reserved_end, 1); }

 protected:
  // FIXME: Unused
  // Marks an object with a special bit field in the header,
  // corresponding to the target region for its whole data-structure graph
  void mark_lag1_object(oop obj);

 public:
  
  LAG1ParMarkDSClosure(G1CollectedHeap * g1,
                       HeapWord * g1h_reserved_end,
                       G1ParScanThreadState * par_scan_state) :
    G1ParClosureSuper(g1, par_scan_state),
    _ds_scanner(g1, par_scan_state), _g1h_reserved_end(g1h_reserved_end)
    { }
  
  template <class T> void do_oop_nv(T * p) { do_oop_work(p); }
  virtual void do_oop(oop * p)       { do_oop_nv(p); }
  virtual void do_oop(narrowOop * p) { do_oop_nv(p); }
};

class LAG1ParMarkFollowerClosure : public G1ParClosureSuper
{
  // Scanner of data-structure fields
  LAG1ParScanDSClosure _ds_scanner;
  
  template <class T> void do_oop_work(T * p, uint32_t m);

 public:
  LAG1ParMarkFollowerClosure(G1CollectedHeap * g1h,
                             G1ParScanThreadState * pss)
    : G1ParClosureSuper(g1h, pss), _ds_scanner(g1h, pss) { }
  
  template <class T> void do_oop_nv(T * p, uint32_t m) { do_oop_work(p, m); }
  template <class T> void do_oop_nv(T * p)             { do_oop_work(p, 0); }
  virtual void do_oop(oop * p)                         { do_oop_nv(p, 0);   }
  virtual void do_oop(narrowOop * p)                   { do_oop_nv(p, 0);   }
};

#endif // SHARE_VM_LAG1_LAG1OOPCLOSURES_HPP
