import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

UNROLL_RESULT_CSV_PATH = "unroll-result.csv"
UNROLL_RESULT_JSON_PATH = "unroll-result.json"
UNROLL_RESULT_PLOT_PDF = "unroll-result.pdf"

def plot_results(csvpath: str, outpath: str) -> None:
  """Plot the results of unroll factor exploration."""
  # Read results into a Pandas DataFrame from CSV.
  df = pd.read_csv(csvpath)

  df["ff"] /= df["ff"][0]
  df["lut"] /= df["lut"][0]

  # Compute resource efficiency, for each ff and lut
  df["ff_efficiency"] = 1000 / (df["latency"] * df["ff"]) 
  df["lut_efficiency"] = 1000 / (df["latency"] * df["lut"])

  fig, host = plt.subplots(1, 2, figsize=(15, 7))
  par1 = host[0].twinx()

  index = np.arange(len(df.ff), dtype=np.float32)
  xticks = [1, 2, 4, 8, 16, 32]

  host[0].set_xticks(index, xticks)
  host[0].set_title("Resource usage and execution time")
  host[0].set_xlabel("unroll factor")
  host[0].set_ylabel("resource usage (relative to non-unroll)")
  par1.set_ylabel("execution time (ns)")
  bar1 = host[0].bar(index - 0.1, df.ff, 0.2, color='lightsteelblue', label="ff_usage")
  bar2 = host[0].bar(index + 0.1, df.lut, 0.2, color='bisque', label='lut_usage')

  lat_line = par1.plot(index, df.latency, color='dimgrey', label='latency', linewidth=2.0)

  h1, l1 = host[0].get_legend_handles_labels()
  h2, l2 = par1.get_legend_handles_labels()
  host[0].legend(h1+h2, l1+l2)

  host[1].set_xticks(index, xticks)
  host[1].set_title("Resource efficiency")
  host[1].set_xlabel("unroll factor")
  host[1].set_ylabel("resource efficiency (relative to non-unroll)")
  host[1].yaxis.set_label_position('right')
  host[1].yaxis.tick_right()
  ff_eff_line = host[1].plot(index, df.ff_efficiency, label='FF efficiency', color='royalblue', linewidth=2.0)
  lue_eff_line = host[1].plot(index, df.lut_efficiency, label='LUT efficiency', color='orange', linewidth=2.0)

  h3, l3 = host[1].get_legend_handles_labels()

  host[1].legend()

  plt.savefig(outpath)
  print(f"Saved plot to {outpath}.")