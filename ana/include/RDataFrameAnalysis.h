/*****************************************************
 * File: RDataFrameAnalysis.h
 * Description: Header file for RDataFrame analysis class
 * Author: Kilo Code
 * Created: 2026/01/21
 ****************************************************/

#ifndef RDATAFRAME_ANALYSIS_H
#define RDATAFRAME_ANALYSIS_H

#include "ConfigReader.h"
#include "ROOT/RDataFrame.hxx"
#include "TChain.h"
#include <map>
#include <memory>
#include <string>

// Forward declarations
class TCutG;
class TH1D;

/**
 * @brief RDataFrame analysis class
 *
 * This class manages the RDataFrame analysis workflow, including:
 * - Configuration loading via ConfigReader
 * - TChain initialization and management
 * - Data analysis using RDataFrame
 * - Visualization and statistics
 */
class RDataFrameAnalysis {
public:
  /**
   * @brief Constructor
   * @param configReader Reference to ConfigReader instance
   */
  explicit RDataFrameAnalysis(ConfigReader &configReader);

  /**
   * @brief Destructor
   */
  ~RDataFrameAnalysis();

  /**
   * @brief Initialize TChain with configuration
   * @return true on success, false on failure
   */
  bool InitializeTChain();

  /**
   * @brief Run the complete analysis workflow
   * @param analysisType String specifying which analysis function to run (e.g.,
   * "GetGammaFlash", "GetThR1")
   * @return true on success, false on failure
   */
  virtual bool RunAnalysis(const std::string &analysisType);

  /**
   * @brief Get the TChain pointer
   * @return Pointer to TChain (may be nullptr if not initialized)
   */
  TChain *GetTChain();

  /**
   * @brief Get the configuration reader
   * @return Reference to ConfigReader
   */
  const ConfigReader &GetConfigReader() const;

  /**
   * @brief Print analysis header
   */
  static void PrintHeader();

  /**
   * @brief Print analysis footer
   */
  static void PrintFooter();

protected:
  /**
   * @brief Analyze data using RDataFrame
   */
  void AnalyzeWithRDataFrame();

  /**
   * @brief Display basic statistics
   * @param df RDataFrame reference
   */
  void DisplayBasicStatistics(ROOT::RDataFrame &df);

  /**
   * @brief Display channel statistics
   * @param df RDataFrame reference
   */
  void DisplayChannelStatistics(ROOT::RDataFrame &df);

  /**
   * @brief Display time range statistics
   * @param df RDataFrame reference
   */
  void DisplayTimeRangeStatistics(ROOT::RDataFrame &df);

  /**
   * @brief Display first few events
   * @param df RDataFrame reference
   * @param numEvents Number of events to display
   */
  void DisplayFirstEvents(ROOT::RDataFrame &df, int numEvents = 5);

  /**
   * @brief Create and display plots
   * @param df RDataFrame reference
   */
  void DisplayPlots(ROOT::RDataFrame &df);

  void GetGammaFlash();
  void GetThR1();
  void GetThR2();
  void GetReactionRate();
  void GetDtForCalL();
  void CalFlightPath();
  void GetPileupCorr();
  void CoincheckSingleBunch();
  void CoincheckDoubleBunch();
  void Coincheck();
  void CountT0();
  void GetHRateXSUF();
  void EvalDeltaTc1();
  void CalSimTrans();

  /**
   * @brief 计算不确定度(纯虚函数)
   *
   * 派生类必须实现此方法以计算各自分析类型的不确定度。
   * 例如:
   * - CrossSectionAnalysis: 计算截面测量的不确定度
   * - NeutronFluxAnalysis: 计算中子通量的不确定度
   */
  virtual void CalUncertainty() = 0;

  // ========== 辅助方法 ==========

  // 初始化辅助方法
  /**
   * @brief 初始化常用配置缓存
   */
  void InitializeCommonConfig();

  /**
   * @brief 初始化能量分箱配置
   */
  void InitializeEnergyBins();

  /**
   * @brief 加载所有通道的 gamma cuts
   * @return true on success, false on failure
   */
  bool LoadGammaCuts();

  /**
   * @brief 加载 ENDF 截面数据
   * @return true on success, false on failure
   */
  bool LoadSTDENDFData(int nrebin = 1);

  /**
   * @brief 加载实验样品 ENDF 截面数据
   * @return true on success, false on failure
   */
  virtual bool LoadXSENDFData(int nrebin = 1);

  // 输出辅助方法
  /**
   * @brief 打印章节标题
   * @param title 标题文本
   */
  void PrintSectionHeader(const std::string &title);

  /**
   * @brief 打印关闭提示信息
   */
  void PrintClosePrompt();

  /**
   * @brief 打印通道处理信息
   * @param chID 通道ID
   */
  void PrintChannelInfo(int chID);

  // RDataFrame 辅助方法
  /**
   * @brief 创建 RDataFrame
   * @return RDataFrame instance
   */
  ROOT::RDataFrame CreateDataFrame();

  /**
   * @brief 启用多线程
   */
  void EnableMultiThreading();

  // 过滤器辅助方法
  /**
   * @brief 按通道ID过滤
   * @param df RDataFrame node
   * @param chID 通道ID
   * @return Filtered RDataFrame node
   */
  ROOT::RDF::RNode FilterByChannel(ROOT::RDF::RNode df, int chID);

  /**
   * @brief 按阈值过滤
   * @param df RDataFrame node
   * @param threshold 阈值
   * @return Filtered RDataFrame node
   */
  ROOT::RDF::RNode FilterByThreshold(ROOT::RDF::RNode df, double threshold);

  /**
   * @brief 应用 gamma cut 过滤
   * @param df RDataFrame node
   * @param chID 通道ID
   * @return Filtered RDataFrame node
   */
  ROOT::RDF::RNode FilterByGammaCut(ROOT::RDF::RNode df, int chID);

  // 列定义辅助方法
  /**
   * @brief 定义 TOF 列
   * @param df RDataFrame node
   * @param Tg Gamma flash time
   * @param Length Flight path length
   * @return RDataFrame node with TOF column
   */
  ROOT::RDF::RNode DefineTOF(ROOT::RDF::RNode df, double Tg, double Length);

  /**
   * @brief 定义能量列
   * @param df RDataFrame node (must have 'tof' column)
   * @param Length Flight path length
   * @return RDataFrame node with En column
   */
  ROOT::RDF::RNode DefineEnergy(ROOT::RDF::RNode df, double Length);

  /**
   * @brief 同时定义 TOF 和能量列
   * @param df RDataFrame node
   * @param Tg Gamma flash time
   * @param Length Flight path length
   * @return RDataFrame node with TOF and En columns
   */
  ROOT::RDF::RNode DefineTOFAndEnergy(ROOT::RDF::RNode df, double Tg,
                                      double Length);

  // 画布辅助方法
  /**
   * @brief 生成画布名称
   * @param prefix 前缀
   * @param chID 通道ID
   * @return Canvas name string
   */
  std::string MakeCanvasName(const std::string &prefix, int chID);

  /**
   * @brief 生成画布标题
   * @param title 标题
   * @param chID 通道ID
   * @return Canvas title string
   */
  std::string MakeCanvasTitle(const std::string &title, int chID);

  // ========== 成员变量 ==========

  ConfigReader &m_configReader; ///< Reference to configuration reader
  TChain *m_chain;              ///< TChain pointer for data access
  bool m_isInitialized;         ///< Initialization status flag

  // 常用配置缓存
  const FIXMConfig *m_fixmConfig; ///< FIXM配置引用
  std::vector<int> m_channelIDs;  ///< 通道ID列表
  std::string m_outputPath;       ///< 当前分析输出数据路径
  std::string m_expName;          ///< 实验名称

  const FIXMConfig *m_lisiConfig{nullptr}; /// LiSi配置引用
  std::vector<int> m_lisiChannelIDs;       /// LiSi通道ID列表
  std::string m_fixmOutputPath;            /// FIXM 输出数据路径
  std::string m_lisiOutputPath;            /// LiSi 输出数据路径

  /// @brief 判断是否有 LiSi 配置可用
  bool HasLiSiConfig() const { return m_lisiConfig && !m_lisiChannelIDs.empty(); }
  /// @brief 临时将 m_fixmConfig/m_channelIDs/m_outputPath 切换至 LiSi
  void SwitchToLiSi();
  /// @brief 恢复到切换前的 FIXM 状态
  void RestoreFromLiSi();
  // 切换前保存的 FIXM 状态
  const FIXMConfig *m_savedConfig{nullptr};
  std::vector<int> m_savedChannelIDs;
  std::string m_savedOutputPath;

  // 能量分箱配置
  int m_bpd;                    ///< Bins per decade
  int m_nDec;                   ///< Number of decades
  int m_nbins;                  ///< Total number of bins
  double m_lowEdge;             ///< Low edge of energy bins
  std::vector<double> m_EnBins; ///< Energy bin edges

  // TCutG 缓存
  std::map<int, TCutG *> m_cutgMap; ///< Gamma cuts for each channel

  // ENDF 截面数据缓存
  std::map<std::string, TGraph *> m_xs_nr;   ///< Reaction cross section data
  std::map<std::string, TGraph *> m_xs_ntot; ///< Total cross section data
  std::map<std::string, TH1D *>
      m_hxs_nr; ///< Reaction cross section data in TH1D

  // Delta L 缓存
  std::shared_ptr<TGraph> m_endeltaL; ///< Delta L graph
  double m_avgDL{0.0};                ///< Average Delta L

  // 常用字符串常量
  static constexpr const char *SEPARATOR =
      "========================================";
  static constexpr const char *CLOSE_PROMPT =
      "Close plot windows to continue...";
  static constexpr const char *EXIT_PROMPT =
      "Press Ctrl+C in terminal to exit...";
};

#endif // RDATAFRAME_ANALYSIS_H
