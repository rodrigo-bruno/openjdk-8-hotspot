#ifndef SHARE_VM_NG2C_VM_OPERATIONS_NG2C_HPP
#define SHARE_VM_NG2C_VM_OPERATIONS_NG2C_HPP

# include "runtime/vm_operations.hpp"
# include "ng2c/ng2c_globals.hpp"

class NG2C_MergeAllocCounters;
class NG2C_MergeJavaThreads;
class NG2C_MergeWorkerThreads;

class NG2C_MergeJavaThreads : public ThreadClosure
{
 private:
  NG2C_MergeAllocCounters * _op;

 public:

  NG2C_MergeJavaThreads(NG2C_MergeAllocCounters * op) : _op(op) { }
  void do_thread(Thread * thread);
};

class NG2C_MergeWorkerThreads : public ThreadClosure
{
 private:
  NG2C_MergeAllocCounters * _op;

 public:
  NG2C_MergeWorkerThreads(NG2C_MergeAllocCounters * op) : _op(op) { }

  void do_thread(Thread * thread);
};

class NG2C_MergeAllocCounters : public VM_Operation
{
 // Note: necessary so that these closures can call private methods.
 friend class NG2C_MergeJavaThreads;
 friend class NG2C_MergeWorkerThreads;

 private:
  static uint * _swp_counter_arr;
  static uint * _inc_counter_arr;
  static uint   _total_update_target_gen;

  // TODO - this methods should be part of closures not the operation...
  void update_promotions(NGenerationArray * global, NGenerationArray * survivors);
  void update_promotions(WorkerThread * thread);
  void update_allocations();
  void increment_allocations(JavaThread* thread);
  void update_target_gen();

 public:
  // There is no need to call the set_calling_thread since the VMThread::execute(&op)
  // will take care of it.
  NG2C_MergeAllocCounters() {
     if (_swp_counter_arr == NULL) {
      _swp_counter_arr = NEW_C_HEAP_ARRAY(uint, NG2C_MAX_ALLOC_SITE, mtGC);
      memset((void*)_swp_counter_arr, 0, NG2C_MAX_ALLOC_SITE * sizeof(uint));
    }
    if (_inc_counter_arr == NULL) {
      _inc_counter_arr = NEW_C_HEAP_ARRAY(uint, NG2C_MAX_ALLOC_SITE, mtGC);
      memset((void*)_inc_counter_arr, 0, NG2C_MAX_ALLOC_SITE * sizeof(uint));
    }
  }

  virtual void doit()
  {
    assert (!calling_thread()->is_VM_thread(), "should not be called by VMThread.");
    // TODO - print how much time is spent on each of these steps.
    {
      NG2C_MergeJavaThreads mjt_cljr(this);
      MutexLocker mu(Threads_lock);
      Threads::threads_do(&mjt_cljr);
    }


    update_allocations();

    {
      NG2C_MergeWorkerThreads mwt_cljr(this);
      MutexLocker mu(Threads_lock);
      Threads::threads_do(&mwt_cljr);
    }

    // Only update target gen every NG2C_GEN_ARRAY_SIZE gc cycles.
    _total_update_target_gen++;
    if (_total_update_target_gen % NG2CUpdateThreshold == 0) {
      update_target_gen();
    }

#ifdef DEBUG_NG2C_PROF_VMOP
    MethodBciHashtable * hashtable = Universe::method_bci_hashtable();

    for (int i = 0; i < hashtable->table_size(); i++) {
      MethodBciEntry * p = (MethodBciEntry*)hashtable->bucket(i);

      for (; p != NULL; p = p->next()) {
        ngen_t * arr = p->literal()->array();
        volatile long * target_gen = p->literal()->target_gen_addr();
        gclog_or_tty->print("[ng2c-vmop] <global hashtable> hash=%u target_gen=%u [",
           p->literal()->hash(), *target_gen);

        for (int k = 0; k < NG2C_GEN_ARRAY_SIZE; k++)
          gclog_or_tty->print(INT64_FORMAT "; ", arr[k]);

        gclog_or_tty->print_cr("]");
      }
    }
#endif
  }

  virtual bool doit_prologue();

  virtual VMOp_Type type() const          { return VMOp_NG2CMergeAllocCounters; }
  virtual Mode evaluation_mode() const    { return _concurrent; }
  virtual bool is_cheap_allocated() const { return true; }

};

#endif // SHARE_VM_NG2C_VM_OPERATIONS_NG2C_HPP