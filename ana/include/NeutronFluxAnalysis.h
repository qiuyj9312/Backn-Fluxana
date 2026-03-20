/*****************************************************
 * File: NeutronFluxAnalysis.h
 * Description: Neutron flux analysis derived class
 * Author: Kilo Code
 * Created: 2026/02/08
 ****************************************************/

#ifndef NEUTRON_FLUX_ANALYSIS_H
#define NEUTRON_FLUX_ANALYSIS_H

#include "RDataFrameAnalysis.h"

/**
 * @brief Neutron flux analysis class
 *
 * Inherits from RDataFrameAnalysis for neutron flux related analysis.
 * All analysis methods are inherited from the base class.
 * Provides semantic classification for flux-related analyses.
 */
class NeutronFluxAnalysis : public RDataFrameAnalysis {
public:
  /**
   * @brief Constructor
   * @param configReader Reference to ConfigReader instance
   */
  explicit NeutronFluxAnalysis(ConfigReader &configReader);

  /**
   * @brief Destructor
   */
  virtual ~NeutronFluxAnalysis();

  /**
   * @brief Run analysis (directly calls base class implementation)
   * @param analysisType Type of analysis to run
   * @return true on success, false on failure
   */
  bool RunAnalysis(const std::string &analysisType) override;

  /**
   * @brief 计算中子通量不确定度
   *
   * 实现基类纯虚函数,计算中子通量测量的各项不确定度:
   * - 统计不确定度 (counting statistics)
   * - 展开不确定度 (double-bunch unfolding)
   * - ENDF 截面不确定度 (cross-section)
   * - 探测器一致性不确定度 (detector consistency)
   * - 总不确定度 (total uncertainty)
   *
   * 输出文件:
   * - herror.root: 包含各通道和样品类型的不确定度直方图
   * - <SampleType>.dat: 包含能量、通量和不确定度的文本文件
   */
  void CalUncertainty() override;

  /**
   * @brief 计算中子通量
   *
   * 从反应率数据计算中子通量,包括:
   * - 合并低能区和高能区数据
   * - 应用探测器效率修正
   * - 应用通量衰减修正
   * - 按样品类型聚合结果
   * - 归一化到束流功率
   * - 设置不确定度
   *
   * 输出文件:
   * - Flux.root: 包含各样品类型的通量直方图
   * - <SampleType>_Flux.dat: 文本格式的通量数据
   */
  void CalFlux();

private:
  // CalUncertainty 辅助函数
  struct UncertaintyData {
    std::map<int, TH1D *> hratem;
    std::map<int, TH1D *> error_rate;
    std::map<int, TH1D *> error_uf;
    std::map<int, TString> sampletype;
    std::map<int, TString> samplename;
    std::map<TString, TH1D *> error_ENDF;
    std::map<TString, TH1D *> error_coin;
    std::map<TString, TH1D *> tot_error_rate;
    std::map<TString, TH1D *> tot_error_uf;
    std::map<TString, TH1D *> tot_error;
    std::map<TString, TH1D *> tot_hrate;
    std::set<TString> sampletype_set;
  };

  void LoadRateHistograms(UncertaintyData &data, const std::string &outcomePath,
                          const std::vector<int> &channelIDs);
  void CalculateStatisticalUncertainty(UncertaintyData &data);
  void CalculateUnfoldingUncertainty(UncertaintyData &data,
                                     const std::string &outcomePath,
                                     const std::vector<int> &channelIDs);
  void LoadDetectorUncertainty(UncertaintyData &data,
                               const std::string &outcomePath);
  void LoadENDFUncertainty(UncertaintyData &data);
  void CalculateTotalUncertainty(UncertaintyData &data,
                                 const std::vector<int> &channelIDs);
  void WriteResults(const UncertaintyData &data,
                    const std::string &outcomePath);
  void DrawUncertaintyPlots(const UncertaintyData &data);

  // CalFlux 辅助函数
  struct FluxData {
    std::map<int, TH1D *> hratexs;       // 合并后的反应率直方图
    std::map<int, TString> sampletype;   // 通道对应的样品类型
    std::map<int, TString> samplename;   // 通道对应的样品名称
    std::map<int, double> nd;            // 通道对应的面积密度
    std::map<int, int> detID;            // 通道对应的探测器ID
    std::set<TString> sampletype_set;    // 所有样品类型集合
    std::map<TString, TH1D *> hflux_tot; // 按样品类型聚合的通量
    std::map<TString, TH1D *> herror;    // 不确定度直方图
  };

  void LoadFluxInputData(FluxData &data, const std::string &outcomePath,
                         const std::vector<int> &channelIDs);
  void LoadLiSiFluxInputData(FluxData &data, const std::string &lisiOutcomePath);
  void CalculateFluxByType(FluxData &data, const std::vector<int> &channelIDs,
                           double expTime, double beamPower,
                           double effectiveArea, int bpd);
  void LoadFluxUncertainty(FluxData &data, const std::string &outcomePath);
  void WriteFluxResults(const FluxData &data, const std::string &outcomePath);
  void DrawFluxPlots(const FluxData &data);
};

#endif // NEUTRON_FLUX_ANALYSIS_H
