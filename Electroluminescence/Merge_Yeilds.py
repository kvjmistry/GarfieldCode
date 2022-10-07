import glob
import pandas as pd
import sys

# To run
# python Merge_Yeilds.py "<path to files to merge>" "<Mode>"

filewildcard = sys.argv[1]
output_name = sys.argv[2]

files = glob.glob(filewildcard)

print(files)

df_concat = pd.DataFrame()

for f in files:
    df = pd.read_hdf(f)
    df_concat = pd.concat([df_concat, df])

df_concat.to_hdf(f"Yields_Merge_{output_name}.h5", "Yields", mode = "w")