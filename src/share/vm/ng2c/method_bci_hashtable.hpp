#ifndef SHARE_VM_NG2C_METHOD_BCI_HASHTABLE_HPP
#define SHARE_VM_NG2C_METHOD_BCI_HASHTABLE_HPP

# include "ng2c/ng2c_globals.hpp"
# include "oops/method.hpp"
# include "utilities/hashtable.hpp"

class MethodBciEntry : public HashtableEntry<NGenerationArray*, mtGC>
{
 public:
  MethodBciEntry * next() const {
    return (MethodBciEntry*)HashtableEntry<NGenerationArray*, mtGC>::next();
  }

  MethodBciEntry ** next_addr() {
    return (MethodBciEntry**)HashtableEntry<NGenerationArray*, mtGC>::next_addr();
  }
};

class MethodBciHashtable : public Hashtable<NGenerationArray*, mtGC>
{
 public:

  MethodBciHashtable (int table_size);

  ngen_t             add_entry (Method * m, int bci);
  NGenerationArray * get_entry(uint hash);
  ngen_t           * get_target_gen(uint hash);
  void               apply_delta (NGenerationArray ** gclocal_ngen_arrays, int sz);
  static unsigned int calculate_hash(Method * m, int bci);
  
};

#endif // SHARE_VM_NG2C_METHOD_BCI_HASHTABLE_HPP