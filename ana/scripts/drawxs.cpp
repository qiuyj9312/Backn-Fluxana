/*
 * @Description: 绘制截面对比图：实验测量数据 vs 评价库数据
 * @Author: Qiu Yijia
 * @Date: 2026-01-24
 */

#include "TCanvas.h"
#include "TFile.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TLegend.h"
#include "TMultiGraph.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

// 颜色数组
EColor color[] = {kRed,    kBlue, kBlack,  kGreen,  kYellow, kMagenta, kCyan,
                  kOrange, kPink, kSpring, kViolet, kTeal,   kAzure};

// 从文件读取图形数据（从 utils.h 复制并简化）
int load_graph(const char *filename, TGraph *graph_, double times = 1.) {
  std::ifstream fin_txt(filename);
  if (!fin_txt.is_open()) {
    std::cerr << "无法打开文件: " << filename << std::endl;
    return -1;
  }

  TString line{};
  line.ReadLine(fin_txt);
  while (!fin_txt.eof()) {
    line.ReadLine(fin_txt);

    Double_t En{}, XS{};

    if (line.Sizeof() < 2) {
      break;
    }
    if (line.Contains("#")) {
      continue;
    }
    if (line.Contains("/")) {
      continue;
    }
    std::stringstream ss(line.Data());
    ss >> En >> XS;
    En = En * times;

    graph_->AddPoint(En, XS);
  }
  return 1;
}

// 从CSV文件读取实验数据并计算总误差
int load_csv_data(const char *filename, TGraphErrors *graph_) {
  std::ifstream fin_csv(filename);
  if (!fin_csv.is_open()) {
    std::cerr << "无法打开CSV文件: " << filename << std::endl;
    return -1;
  }

  std::string header;
  std::getline(fin_csv, header);

  // 解析表头,找到各列的位置
  std::stringstream header_ss(header);
  std::string col_name;
  std::vector<std::string> col_names;

  while (std::getline(header_ss, col_name, ',')) {
    col_names.push_back(col_name);
  }

  // 查找关键列的索引
  int idx_data = -1, idx_en = -1;
  int idx_err_s = -1, idx_err_sys = -1, idx_err_t = -1;

  for (size_t i = 0; i < col_names.size(); i++) {
    if (col_names[i].find("DATA (B)") != std::string::npos) {
      idx_data = i;
    } else if (col_names[i].find("EN (EV)") != std::string::npos) {
      idx_en = i;
    } else if (col_names[i].find("ERR-S") != std::string::npos) {
      idx_err_s = i;
    } else if (col_names[i].find("ERR-SYS") != std::string::npos) {
      idx_err_sys = i;
    } else if (col_names[i].find("ERR-T") != std::string::npos) {
      idx_err_t = i;
    }
  }

  if (idx_data < 0 || idx_en < 0) {
    std::cerr << "错误: 未找到必需的列 (DATA或EN)" << std::endl;
    fin_csv.close();
    return -1;
  }

  int nPoints = 0;
  std::string line;

  while (std::getline(fin_csv, line)) {
    if (line.empty())
      continue;

    std::stringstream ss(line);
    std::string item;
    std::vector<std::string> tokens;

    while (std::getline(ss, item, ',')) {
      tokens.push_back(item);
    }

    if ((int)tokens.size() <= std::max(idx_data, idx_en))
      continue;

    try {
      double xs = std::stod(tokens[idx_data]);
      double en = std::stod(tokens[idx_en]);

      double total_err = 0.0;

      // 优先使用ERR-T(总误差)
      if (idx_err_t >= 0 && (int)tokens.size() > idx_err_t &&
          !tokens[idx_err_t].empty()) {
        double err_t = std::stod(tokens[idx_err_t]);
        // 判断是百分比还是绝对值
        if (col_names[idx_err_t].find("PER-CENT") != std::string::npos) {
          total_err = xs * err_t / 100.0;
        } else {
          total_err = err_t;
        }
      } else {
        // 否则,组合统计和系统误差
        double err_stat = 0.0, err_sys = 0.0;

        if (idx_err_s >= 0 && (int)tokens.size() > idx_err_s &&
            !tokens[idx_err_s].empty()) {
          err_stat = std::stod(tokens[idx_err_s]);
        }

        if (idx_err_sys >= 0 && (int)tokens.size() > idx_err_sys &&
            !tokens[idx_err_sys].empty()) {
          err_sys = std::stod(tokens[idx_err_sys]);
        }

        // 判断误差格式
        bool err_s_is_percent =
            (idx_err_s >= 0 &&
             col_names[idx_err_s].find("PER-CENT") != std::string::npos);
        bool err_sys_is_percent =
            (idx_err_sys >= 0 &&
             col_names[idx_err_sys].find("PER-CENT") != std::string::npos);

        // 转换为绝对值
        double abs_err_stat =
            err_s_is_percent ? (xs * err_stat / 100.0) : err_stat;
        double abs_err_sys =
            err_sys_is_percent ? (xs * err_sys / 100.0) : err_sys;

        total_err =
            std::sqrt(abs_err_stat * abs_err_stat + abs_err_sys * abs_err_sys);
      }

      graph_->SetPoint(nPoints, en, xs);
      graph_->SetPointError(nPoints, 0, total_err);
      nPoints++;

    } catch (const std::exception &e) {
      continue;
    }
  }

  fin_csv.close();
  return nPoints;
}

void drawxs() {
  // ========================================
  // 1. 定义评价库数据文件路径
  // ========================================
  const char *evalDataFiles[] = {
      "/home/qyj/work/XSana/XS/2025_232Th/para/CENDL-3.2.txt",
      "/home/qyj/work/XSana/XS/2025_232Th/para/ENDFB-VIII.1.txt",
      "/home/qyj/work/XSana/XS/2025_232Th/para/JEFF-4.0.txt",
      "/home/qyj/work/XSana/XS/2025_232Th/para/JENDL-5.txt",
      "/home/qyj/work/XSana/XS/2025_232Th/para/ROSFOND-2010.txt",
      "/home/qyj/work/XSana/XS/2025_232Th/para/ADS-HE.txt",
      "/home/qyj/work/XSana/XS/2025_232Th/para/TENDL-2023.txt"};

  const char *evalDataNames[] = {"CENDL-3.2", "ENDF/B-VIII.1", "JEFF-4.0",
                                 "JENDL-5",   "ROSFOND-2010",  "ADS-HE",
                                 "TENDL-2023"};

  const int nEvalData = 7;

  // ========================================
  // 2. 从 XSSimple.root 读取实验测量数据
  // ========================================
  TFile *f =
      TFile::Open("/home/qyj/work/XSana/XS/2025_232Th/Outcome/XSSimple.root");
  if (!f || f->IsZombie()) {
    std::cerr << "Error: Cannot open XSSimple.root" << std::endl;
    return;
  }

  std::cout << "========================================" << std::endl;
  std::cout << "成功打开 XSSimple.root" << std::endl;

  // 获取 hxs 直方图
  TH1D *hxs = nullptr;
  f->GetObject("hxs", hxs);
  if (!hxs) {
    std::cerr << "Error: Cannot find hxs histogram in file" << std::endl;
    f->Close();
    return;
  }
  hxs->Scale(0.92);
  std::cout << "成功读取 hxs 直方图" << std::endl;
  std::cout << "  - Bins: " << hxs->GetNbinsX() << std::endl;
  std::cout << "  - Entries: " << hxs->GetEntries() << std::endl;

  // ========================================
  // 3. 将 hxs 转换为 TGraphErrors
  // ========================================
  TGraphErrors *grExp = new TGraphErrors();
  grExp->SetName("grExp");
  grExp->SetTitle("Experimental Data");

  int nPoints = 0;
  for (int i = 1; i <= hxs->GetNbinsX(); i++) {
    double x = hxs->GetBinCenter(i);
    double y = hxs->GetBinContent(i);
    double ex = hxs->GetBinWidth(i) / 2.0;
    double ey = hxs->GetBinError(i);

    // 只添加有数据的点
    if (y > 0) {
      grExp->SetPoint(nPoints, x, y);
      grExp->SetPointError(nPoints, ex, ey);
      nPoints++;
    }
  }

  std::cout << "实验数据点数: " << nPoints << std::endl;

  // ========================================
  // 3.5. 将实验数据导出到 txt 文件
  // ========================================
  const char *expDataFile =
      "/home/qyj/work/XSana/XS/2025_232Th/Outcome/ExperimentalData.txt";
  std::ofstream fout(expDataFile);
  if (!fout.is_open()) {
    std::cerr << "Error: Cannot create output file " << expDataFile
              << std::endl;
  } else {
    // 写入文件头
    fout << "# Experimental Cross Section Data for 232Th(n,f)" << std::endl;
    fout << "# Generated on: " << __DATE__ << " " << __TIME__ << std::endl;
    fout << "# Data extracted from: XSSimple.root/hxs" << std::endl;
    fout << "#" << std::endl;
    fout << "# Columns:" << std::endl;
    fout << "#   1. Energy (eV)" << std::endl;
    fout << "#   2. Cross Section (barn)" << std::endl;
    fout << "#   3. Energy Error (eV)" << std::endl;
    fout << "#   4. Cross Section Error (barn)" << std::endl;
    fout << "#" << std::endl;
    fout << std::scientific << std::setprecision(6);

    // 写入数据点
    for (int i = 0; i < grExp->GetN(); i++) {
      double x, y, ex, ey;
      grExp->GetPoint(i, x, y);
      ex = grExp->GetErrorX(i);
      ey = grExp->GetErrorY(i);

      fout << x << "\t" << y << "\t" << ex << "\t" << ey << std::endl;
    }

    fout.close();
    std::cout << "实验数据已导出至: " << expDataFile << std::endl;
  }

  // 设置实验数据样式
  grExp->SetMarkerStyle(20);
  grExp->SetMarkerSize(0.8);
  grExp->SetMarkerColor(kBlack);
  grExp->SetLineColor(kBlack);

  // ========================================
  // 3.5. 读取CSV格式的实验数据
  // ========================================
  const char *csvDataFiles[] = {
      "/home/qyj/work/XSana/XS/2025_232Th/para/41455013.csv",
      "/home/qyj/work/XSana/XS/2025_232Th/para/236540022.csv",
      "/home/qyj/work/XSana/XS/2025_232Th/para/328730022.csv",
      "/home/qyj/work/XSana/XS/2025_232Th/para/328730032.csv",
      "/home/qyj/work/XSana/XS/2025_232Th/para/328870022.csv",
      "/home/qyj/work/XSana/XS/2025_232Th/para/328870032.csv",
      "/home/qyj/work/XSana/XS/2025_232Th/para/328890022.csv",
      "/home/qyj/work/XSana/XS/2025_232Th/para/329190022.csv"};

  const char *csvDataNames[] = {
      "O.Shcherbakov+ 2002",
      "D.Tarrio+ 2023",
      "Yu.M.Gledenov+ 2022 (Mono-energetic Neutron Sources)",
      "Yu.M.Gledenov+ 2022 (White Neutron Sources)",
      "Yonghao Chen+ 2023 (Relative to ^{1}H(n,1))",
      "Yonghao Chen+ 2023 (Relative to ^{235}U(n,f))",
      "Zhizhou Ren+ 2023",
      "Haofan Bai+ 2024"};

  const int nCSVData = 8;

  TGraphErrors *grCSV[nCSVData];
  int nSuccessCSV = 0;

  std::cout << "\n========================================" << std::endl;
  std::cout << "开始读取CSV格式实验数据..." << std::endl;

  for (int i = 0; i < nCSVData; i++) {
    grCSV[i] = new TGraphErrors();
    grCSV[i]->SetName(Form("grCSV_%d", i));
    grCSV[i]->SetTitle(csvDataNames[i]);

    int npts = load_csv_data(csvDataFiles[i], grCSV[i]);

    if (npts < 0) {
      std::cerr << "Warning: Failed to read " << csvDataFiles[i] << std::endl;
      delete grCSV[i];
      grCSV[i] = nullptr;
    } else {
      std::cout << "成功读取 " << csvDataNames[i] << " (" << npts << " points)"
                << std::endl;

      // 设置不同的样式和颜色(避免与grExp的黑色重复)
      // 使用不同的标记样式和颜色组合
      const int markerStyles[] = {21, 22, 23, 29,
                                  33, 34, 47, 43}; // 8种不同的标记
      const EColor csvColors[] = {kRed,
                                  kBlue,
                                  (EColor)(kGreen + 2),
                                  kMagenta,
                                  (EColor)(kOrange + 1),
                                  (EColor)(kCyan + 2),
                                  kViolet,
                                  (EColor)(kSpring + 2)};

      grCSV[i]->SetMarkerStyle(markerStyles[i]);
      grCSV[i]->SetMarkerSize(0.8);
      grCSV[i]->SetMarkerColor(csvColors[i]);
      grCSV[i]->SetLineColor(csvColors[i]);
      nSuccessCSV++;
    }
  }

  std::cout << "成功读取 " << nSuccessCSV << " 个CSV数据集" << std::endl;

  // ========================================
  // 4. 读取评价库数据
  // ========================================
  TGraph *grEval[nEvalData];
  int nSuccessEval = 0;

  for (int i = 0; i < nEvalData; i++) {
    grEval[i] = new TGraph();
    grEval[i]->SetName(Form("gr_%s", evalDataNames[i]));
    grEval[i]->SetTitle(evalDataNames[i]);

    // 读取数据，能量从 MeV 转换为 eV（乘以 1e6）
    int ret = load_graph(evalDataFiles[i], grEval[i], 1e6);

    if (ret < 0) {
      std::cerr << "Warning: Failed to read " << evalDataFiles[i] << std::endl;
      delete grEval[i];
      grEval[i] = nullptr;
    } else {
      std::cout << "成功读取 " << evalDataNames[i] << " (" << grEval[i]->GetN()
                << " points)" << std::endl;

      // 设置不同的颜色和线型
      grEval[i]->SetLineColor(color[i % 13]);
      grEval[i]->SetLineWidth(2);
      grEval[i]->SetLineStyle(1);
      nSuccessEval++;
    }
  }

  // ========================================
  // 5. 创建 Canvas 并绘制
  // ========================================
  TCanvas *c1 = new TCanvas("c1", "Cross Section Comparison", 1200, 800);
  c1->SetLogy();
  c1->SetLogx();
  c1->SetGrid();

  // 创建 TMultiGraph 用于统一管理所有图形
  TMultiGraph *mg = new TMultiGraph();
  mg->SetTitle("^{232}Th(n,f) Cross Section;Neutron Energy (eV);Cross "
               "Section (barn)");

  // 先添加评价库数据(作为背景)
  for (int i = 0; i < nEvalData; i++) {
    if (grEval[i] != nullptr) {
      mg->Add(grEval[i], "L");
    }
  }
  std::cout << grEval[1]->Eval(14e6) << "\t" << grExp->Eval(14e6) << std::endl;

  // 添加CSV实验数据
  for (int i = 0; i < nCSVData; i++) {
    if (grCSV[i] != nullptr) {
      mg->Add(grCSV[i], "PEZ");
    }
  }

  // 最后添加实验数据(在最上层)
  mg->Add(grExp, "PEZ");

  // 绘制
  mg->Draw("A");

  // 设置坐标轴范围
  mg->GetXaxis()->SetRangeUser(0.99e6, 2.01e8); // 500 keV - 400 MeV
  mg->GetYaxis()->SetRangeUser(1e-5, 1.2);      // 截面范围

  // ========================================
  // 6. 创建图例
  // ========================================
  TLegend *leg = new TLegend(0.12, 0.12, 0.45, 0.88);
  leg->SetFillStyle(0);
  leg->SetBorderSize(1);
  leg->SetTextSize(0.025);

  // 添加实验数据到图例
  leg->AddEntry(grExp, "This Work", "PLE");

  // 添加CSV实验数据到图例
  for (int i = 0; i < nCSVData; i++) {
    if (grCSV[i] != nullptr) {
      leg->AddEntry(grCSV[i], csvDataNames[i], "PLE");
    }
  }

  // 添加评价库数据到图例
  for (int i = 0; i < nEvalData; i++) {
    if (grEval[i] != nullptr) {
      leg->AddEntry(grEval[i], evalDataNames[i], "L");
    }
  }

  leg->Draw();

  // ========================================
  // 7. 更新画布并保存
  // ========================================
  c1->Update();

  std::cout << "========================================" << std::endl;

  // ========================================
  // 8. 创建相对误差图
  // ========================================
  std::cout << "\n开始创建相对误差图..." << std::endl;

  // 创建相对误差的 TGraphErrors
  TGraphErrors *grRelErr = new TGraphErrors();
  grRelErr->SetName("grRelErr");
  grRelErr->SetTitle("Relative Error of Experimental Data");

  int nRelErrPoints = 0;
  for (int i = 1; i <= hxs->GetNbinsX(); i++) {
    double x = hxs->GetBinCenter(i);
    double y = hxs->GetBinContent(i);
    double ex = hxs->GetBinWidth(i) / 2.0;
    double ey = hxs->GetBinError(i);

    // 只添加有数据的点，计算相对误差
    if (y > 0) {
      double relErr = ey / y; // 相对误差 = 误差 / 值
      grRelErr->SetPoint(nRelErrPoints, x, relErr * 100); // 转换为百分比
      grRelErr->SetPointError(nRelErrPoints, ex, 0); // Y方向不设置误差
      nRelErrPoints++;
    }
  }

  std::cout << "相对误差数据点数: " << nRelErrPoints << std::endl;

  // 设置相对误差图样式
  grRelErr->SetMarkerStyle(20);
  grRelErr->SetMarkerSize(0.8);
  grRelErr->SetMarkerColor(kBlue);
  grRelErr->SetLineColor(kBlue);
  grRelErr->SetLineWidth(2);

  // 创建新的 Canvas 绘制相对误差
  TCanvas *c2 =
      new TCanvas("c2", "Relative Statistical Uncertainty", 1200, 600);
  c2->SetLogx();
  c2->SetGrid();

  // 绘制相对误差图
  grRelErr->Draw("APL");
  grRelErr->GetXaxis()->SetTitle("Neutron Energy (eV)");
  grRelErr->GetYaxis()->SetTitle("Statistical Uncertainty (%)");
  grRelErr->SetTitle("^{232}Th(n,f) Cross Section - Statistical Uncertainty");
  // grRelErr->GetXaxis()->SetRangeUser(1, 2.5e8); // 500 keV - 250 MeV

  // 设置坐标轴范围
  grRelErr->GetXaxis()->SetLimits(0.99e6, 2.01e8);

  // 添加图例
  TLegend *leg2 = new TLegend(0.65, 0.75, 0.88, 0.88);
  leg2->SetFillStyle(0);
  leg2->SetBorderSize(1);
  leg2->SetTextSize(0.03);
  leg2->SetHeader("^{232}Th(n,f)", "C");
  leg2->AddEntry(grRelErr, "Relative Error", "PL");
  leg2->Draw();

  // 更新画布并保存
  c2->Update();

  std::cout << "\n========================================" << std::endl;
  std::cout << "相对误差图绘制完成！" << std::endl;
  std::cout << "========================================" << std::endl;

  // 不关闭文件，保持交互
  // f->Close();
}
