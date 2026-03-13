#include "TCanvas.h"
#include "TFile.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// 读取数据文件，忽略以'#'开头的注释行
TGraph *ReadGraph(const string &filename) {
  TGraph *g = new TGraph();
  ifstream ifs(filename);
  if (!ifs.is_open()) {
    cerr << "Cannot open " << filename << endl;
    return g;
  }
  string line;
  int n = 0;
  while (getline(ifs, line)) {
    if (line.empty() || line[0] == '#')
      continue;
    double x, y;
    stringstream ss(line);
    if (ss >> x >> y) {
      g->SetPoint(n++, x, y);
    }
  }
  return g;
}

void calc_xs_error() {
  string dir = "/home/qyj/work/XSana/Para/XS/235U/";
  string outTxt = "/home/qyj/work/XSana/Para/XS/235U/xs_error_out.dat";

  vector<string> files = {"BROND-3.1.dat", "CENDL-3.2.dat", "ENDFB-VIII.1.dat",
                          "JEFF-4.0.dat", "JENDL-5.dat"};

  vector<TGraph *> graphs;
  for (const auto &file : files) {
    graphs.push_back(ReadGraph(dir + file));
  }

  int N = graphs.size();
  if (N == 0) {
    cerr << "No graph loaded." << endl;
    return;
  }

  // 设置能量范围 9.4 eV 到 150 eV （注意原文件单位为MeV）
  double E_min = 1e-6; // MeV
  double E_max = 1e-3; // MeV
  int bpd = 100;       // 1000 bins per decade

  double Emax_min = 1e-6;   // MeV
  double Emin_max = 150e-6; // MeV

  double log_E_min = log10(E_min);
  double log_E_max = log10(E_max);

  // 计算在这段对数区间的总点数
  int num_bins = ceil((log_E_max - log_E_min) * bpd);

  vector<double> energies;
  vector<double> mean_xs;
  vector<double> err_mean;

  // 辅助函数，不在源文件外部定义也可以直接内联写在循环中

  // 按对数等距插值计算每个bin内的积分平均值
  for (int i = 0; i < num_bins; ++i) {
    double E1 = pow(10, log_E_min + (double)i / bpd);
    double E2 = pow(10, log_E_min + (double)(i + 1) / bpd);
    if (E1 > E_max)
      break;
    if (E2 > E_max)
      E2 = E_max;

    // 对每个bin内划分若干小步长，利用梯形法则进行数值积分
    int n_steps = 100;
    double step = (E2 - E1) / n_steps;

    vector<double> xs_vals;
    double sum = 0;
    for (int j = 0; j < N; ++j) {
      double integral = 0;
      double val1 = graphs[j]->Eval(E1);
      for (int k = 1; k <= n_steps; ++k) {
        double val2 = graphs[j]->Eval(E1 + k * step);
        integral += (val1 + val2) / 2.0 * step;
        val1 = val2;
      }
      double avg_val =
          integral / (E2 - E1); // 当前TGraph在当前bin内的积分平均值

      xs_vals.push_back(avg_val);
      sum += avg_val;
    }

    double mean = sum / N; // 5个图的积分平均值的平均值
    double sq_diff_sum = 0;
    for (int j = 0; j < N; ++j) {
      sq_diff_sum += (xs_vals[j] - mean) * (xs_vals[j] - mean);
    }

    // 这5个积分平均值的实验标准偏差
    double std_dev_mean = sqrt(sq_diff_sum / (N * (N - 1)));

    // 采用该对数bin的中心值作为代表能量点
    double E_center = pow(10, log_E_min + (i + 0.5) / bpd);
    energies.push_back(E_center);
    mean_xs.push_back(mean);
    err_mean.push_back(std_dev_mean);
  }

  // 结果输出到文本文档
  ofstream ofs(outTxt);
  // 仅输出能量(eV)和偏差(b)
  for (size_t i = 0; i < energies.size(); ++i) {
    if (energies[i] < Emax_min || energies[i] > Emin_max) {
      continue;
    }
    ofs << energies[i] * 1e6 << "\t" << err_mean[i] / mean_xs[i] << endl;
  }
  ofs.close();

  // 画出图像 (能量单位转为eV绘图)
  TCanvas *c1 = new TCanvas("c1", "Deviation of average", 800, 600);
  c1->SetLogx();
  TGraph *g_err = new TGraph(energies.size());
  for (size_t i = 0; i < energies.size(); ++i) {
    g_err->SetPoint(i, energies[i] * 1e6, err_mean[i] / mean_xs[i]);
  }
  g_err->SetTitle("Experimental Standard Deviation of Average XS;Energy "
                  "(eV);Relative Deviation");
  g_err->SetLineColor(kBlue);
  g_err->SetLineWidth(2);
  g_err->Draw("AL");
}
