/*****************************************************
 * File: CrossSectionAnalysis.h
 * Description: Cross section analysis derived class
 * Author: Kilo Code
 * Created: 2026/02/08
 ****************************************************/

#ifndef CROSS_SECTION_ANALYSIS_H
#define CROSS_SECTION_ANALYSIS_H

#include "RDataFrameAnalysis.h"

/**
 * @brief Cross section analysis class
 *
 * Inherits from RDataFrameAnalysis for cross section specific analysis.
 * Currently contains only GetXSSingleBunch() as a derived class specific
 * method.
 */
class CrossSectionAnalysis : public RDataFrameAnalysis {
public:
  /**
   * @brief Constructor
   * @param configReader Reference to ConfigReader instance
   */
  explicit CrossSectionAnalysis(ConfigReader &configReader);

  /**
   * @brief Destructor
   */
  virtual ~CrossSectionAnalysis();

  /**
   * @brief Run analysis
   * @param analysisType Type of analysis to run
   * @return true on success, false on failure
   */
  bool RunAnalysis(const std::string &analysisType) override;

  /**
   * @brief Single bunch cross section calculation (derived class specific
   * method)
   */
  void GetXSSingleBunch();

  /**
   * @brief 加载实验样品 ENDF 截面数据
   * @return true on success, false on failure
   */
  bool LoadXSENDFData(int nrebin = 1) override;

  /**
   * @brief 计算截面测量不确定度
   *
   * 实现基类纯虚函数,计算截面测量的不确定度。
   * TODO: 实现截面不确定度计算逻辑
   */
  void CalUncertainty() override;
};

#endif // CROSS_SECTION_ANALYSIS_H
