import numpy  as np
import pandas as pd
import sys

Mode = sys.argv[1]
option = sys.argv[2]

# sample_type = "unitcell"
sample_type = "dist"

print("Using mode: ", Mode)
print("with Option: ", option)
print("Sampling with: ", sample_type)

# Transform x, y positions to unit cell positions
hexsize = (1.25+0.127/2.0)/(np.cos(30*np.pi/180))

# The hexagon size is larger for the rotated mesh
if (Mode == "Rot30"):
    hexsize = 24.446 # mm

# Define bins in q and r space
numbins = 50

bins_q, bw_q  = np.linspace(-1, +1, numbins, retstep=True)
bins_centre_q = np.linspace(-1+bw_q/2, 1+bw_q/2, numbins)[:-1]

bins_r, bw_r  = np.linspace(-1, +1, numbins, retstep=True)
bins_centre_r = np.linspace(-1+bw_r/2, 1+bw_r/2, numbins)[:-1]

# Calculate the nearest hexagon centre to the point
def hex_round(q,r,s,mode):
    qi = int(round(q))
    ri = int(round(r))
    si = int(round(s))
    q_diff = abs(qi - q)
    r_diff = abs(ri - r)
    s_diff = abs(si - s)
    
    if q_diff > r_diff and q_diff > s_diff:
        qi = -ri - si
    else:
        if r_diff > s_diff:
            ri = -qi - si
        else:
            si = -qi - ri
    
    if (mode == "q"):
        return qi
    elif (mode == "r"):
        return ri
    else:
        return si


def GetHexCoords(df):
    # Calculate q and r values for hits table
    df["q"] = (df["x"] * np.sqrt(3)/3.0 - 1.0/3.0*df["y"]) / hexsize 
    df["r"] = (2.0/3.0)*df["y"] / hexsize
    df["s"] = -df["q"] - df["r"]

    # Calculate the multiple of q and r of the hexagon ~ 50 s
    df["nq"] = df.apply(lambda df_: hex_round(df_["q"], df_["r"], df_["s"], "q"), axis = "columns", result_type='expand' )
    df["nr"] = df.apply(lambda df_: hex_round(df_["q"], df_["r"], df_["s"], "r"), axis = "columns", result_type='expand' )

    # Shift the values to unit cell
    df["q"] = df["q"] - df["nq"]
    df["r"] = df["r"] - df["nr"]

    # Drop columns that are now not needed
    df = df.drop(columns = ["s", "nq","nr", "x", "y"])

    # Now bin the data ~20s
    df['q'] = pd.cut(x=df['q'], bins=bins_q,labels=bins_centre_q, include_lowest=True)
    df['r'] = pd.cut(x=df['r'], bins=bins_r,labels=bins_centre_r, include_lowest=True)
    return df

if (sample_type == "unitcell"): unitcell = pd.read_hdf(f"unitcell_{Mode}.h5","Yields")
if (sample_type == "dist"): unitcell = pd.read_hdf(f"dist_unitcell_{Mode}.h5","Yields")

# unitcell = pd.read_hdf(f"unitcell_{Mode}.h5","Yields")

unitcell = unitcell.drop(columns = ["x", "y"])
unitcell['q'] = unitcell['q'].astype(float)
unitcell['r'] = unitcell['r'].astype(float)
unitcell['excitation'] = unitcell['excitation'].astype(float)


input_file = "NEW.eminus.next.h5"

## True hits (deposited energy)
hits = pd.read_hdf(input_file, 'MC/hits')
hits = hits[hits["label"] == "ACTIVE"]
hits = hits.drop(columns = ["label", "hit_id", "time", "particle_id"])

# Replace nan to zeros
hits['energy'] = hits['energy'].fillna(0)

# Calculate the total energy of each hit
hitsum = hits
hitsum = hitsum.drop(columns = ["x", "y", "z"])
hitsum['Esum'] = hitsum.groupby(["event_id"])["energy"].transform('sum')
hitsum = hitsum.drop(columns = ["energy"])
hitsum = hitsum.drop_duplicates()


if (option == "bb"):
    # Get a list of events who do not deposit all their energy in the detector
    bad_events = hitsum[hitsum["Esum"] != 2.458].event_id.values

    # Filter the main hit list from the bad events
    hits = hits[~hits.event_id.isin(bad_events)]

# Change the units of energy to eV
hits["energy"] = hits["energy"]*1e6
hits["ni"] = hits["energy"]/22.0
hits["ni"] = hits["ni"].round()

# Calculate the transverse diffusion sigma
D_T = 1.07 # mm/cm^0.5
hits["sigma"] = np.sqrt(hits["z"] * 0.1) * D_T # [mm]
print(hits["sigma"].max())
print(hits["sigma"].min())

print("z:", hits["z"].max())
print("z:", hits["z"].min())

print("x:", hits["x"].max())
print("x:", hits["x"].min())

print("y:", hits["y"].max())
print("y:", hits["y"].min())

print(hits["ni"].sum())

# Get rid of hits that do not have any energy deposits
hits = hits[hits["ni"] != 0]

# Drop the energy and z columns
hits = hits.drop(columns = ["energy"])

# Get the average z position of the event
hits['z_mean'] = hits.groupby(["event_id"])["z"].transform('mean')
hits = hits.drop(columns = ["z"])

print(len(hits.event_id.unique()))


rng = np.random.default_rng()

evid = -1

Yieldsum = 0

Yields = []
z_avg = []

for ev,x,y,ni,sigma,z in zip(hits["event_id"], hits["x"], hits["y"], hits["ni"], hits["sigma"], hits["z_mean"]):

    temp_df = pd.DataFrame()

    if (evid != ev):

        if (Yieldsum != 0):
            print("Yield: ",Yieldsum)
            Yields.append(Yieldsum)
            z_avg.append(z)

        Yieldsum = 0
        evid = ev

        print(ev)
    
    # Calculate smeared position for each electron
    mean = (x, y)
    cov = [[sigma, 0], [0, sigma]]
    x_samp, y_samp = rng.multivariate_normal(mean, cov, int(ni)).T

    # Create a dataframe and bin the x and y sample
    temp_df["x"] = x_samp
    temp_df["y"] = y_samp

    # Transform the sampled x,y position to a binned q,r position in the hexagon unit cell
    temp_df = GetHexCoords(temp_df)
    
    # Choose the sample type
    if (sample_type == "unitcell"):
        Yieldsum += pd.merge(temp_df, unitcell, how ='inner', on =['q', 'r']).excitation.sum()
    else:

        # Use distribution method:

        # Loop over the temp df
        for temp_q, temp_r in zip(temp_df["q"], temp_df["r"]):

            # Get part of the unitcell df for a given q and r
            query = unitcell[(unitcell["q"] == temp_q) & (unitcell["r"] == temp_r)]

            bin_width = 20
            bin_centers =  np.arange(810, 1390, bin_width)
            hist, edges = np.histogram(query.excitation.values, bins = np.arange(800, 1400, bin_width), density=True)
            hist = hist*bin_width

            # Sample the distribution
            sample = rng.choice(bin_centers, p = hist)
            Yieldsum += sample


Outdf = pd.DataFrame()
Outdf["Yield"] = Yields
Outdf["z"] = z_avg

Outdf.to_hdf("Yields.h5", "Yields", mode = "w")


