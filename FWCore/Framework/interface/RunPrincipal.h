#ifndef FWCore_Framework_RunPrincipal_h
#define FWCore_Framework_RunPrincipal_h

/*----------------------------------------------------------------------

RunPrincipal: This is the class responsible for management of
per run EDProducts. It is not seen by reconstruction code;
such code sees the Run class, which is a proxy for RunPrincipal.

The major internal component of the RunPrincipal
is the DataBlock.

----------------------------------------------------------------------*/

#include <string>
#include <vector>

#include <memory>

#include "DataFormats/Provenance/interface/RunAuxiliary.h"
#include "DataFormats/Provenance/interface/ProcessHistoryID.h"
#include "FWCore/Utilities/interface/propagate_const.h"
#include "FWCore/Utilities/interface/RunIndex.h"
#include "FWCore/Framework/interface/Principal.h"
#include "FWCore/Framework/interface/ProductResolversFactory.h"

namespace edm {

  class HistoryAppender;
  class MergeableRunProductProcesses;
  class MergeableRunProductMetadata;
  class ModuleCallingContext;
  class ProcessHistoryRegistry;

  class RunPrincipal : public Principal {
  public:
    typedef RunAuxiliary Auxiliary;
    typedef Principal Base;

    template <ProductResolversFactory FACTORY>
    RunPrincipal(std::shared_ptr<ProductRegistry const> reg,
                 FACTORY&& iFactory,
                 ProcessConfiguration const& pc,
                 HistoryAppender* historyAppender,
                 unsigned int iRunIndex,
                 MergeableRunProductProcesses const* mergeableRunProductProcesses = nullptr)
        : RunPrincipal(reg,
                       iFactory(InRun, pc.processName(), *reg),
                       pc,
                       historyAppender,
                       iRunIndex,
                       mergeableRunProductProcesses) {}
    ~RunPrincipal() override;

    void fillRunPrincipal(ProcessHistoryRegistry const& processHistoryRegistry, DelayedReader* reader = nullptr);

    /** Multiple Runs may be processed simultaneously. The
     return value can be used to identify a particular Run.
     The value will range from 0 to one less than
     the maximum number of allowed simultaneous Runs. A particular
     value will be reused once the processing of the previous Run
     using that index has been completed.
     */
    RunIndex index() const { return index_; }

    void setAux(RunAuxiliary iAux) { aux_ = iAux; }
    RunAuxiliary const& aux() const { return aux_; }

    RunNumber_t run() const { return aux().run(); }

    ProcessHistoryID const& reducedProcessHistoryID() const { return m_reducedHistoryID; }

    RunID const& id() const { return aux().id(); }

    Timestamp const& beginTime() const { return aux().beginTime(); }

    Timestamp const& endTime() const { return aux().endTime(); }

    void setEndTime(Timestamp const& time) { aux_.setEndTime(time); }

    void mergeAuxiliary(RunAuxiliary const& aux) { return aux_.mergeAuxiliary(aux); }

    void put(ProductDescription const& bd, std::unique_ptr<WrapperBase> edp) const;

    void put(ProductResolverIndex index, std::unique_ptr<WrapperBase> edp) const;

    void putOrMerge(ProductDescription const& bd, std::unique_ptr<WrapperBase> edp) const;

    MergeableRunProductMetadata* mergeableRunProductMetadata() { return mergeableRunProductMetadataPtr_.get(); }

    void preReadFile();

    enum ShouldWriteRun { kUninitialized, kNo, kYes };
    ShouldWriteRun shouldWriteRun() const { return shouldWriteRun_; }
    void setShouldWriteRun(ShouldWriteRun value) { shouldWriteRun_ = value; }

  private:
    RunPrincipal(std::shared_ptr<ProductRegistry const> reg,
                 std::vector<std::shared_ptr<ProductResolverBase>>&& resolvers,
                 ProcessConfiguration const& pc,
                 HistoryAppender* historyAppender,
                 unsigned int iRunIndex,
                 MergeableRunProductProcesses const* mergeableRunProductProcesses);
    unsigned int transitionIndex_() const override;

    RunAuxiliary aux_;
    ProcessHistoryID m_reducedHistoryID;
    RunIndex index_;

    // For the primary input RunPrincipals created by the EventProcessor,
    // there should be one MergeableRunProductMetadata object created
    // per concurrent run. In all other cases, this should just be null.
    edm::propagate_const<std::unique_ptr<MergeableRunProductMetadata>> mergeableRunProductMetadataPtr_;

    ShouldWriteRun shouldWriteRun_ = kUninitialized;
  };
}  // namespace edm
#endif
