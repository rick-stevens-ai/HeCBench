import numpy as np

# Generate synthetic data: 500000 points with 5 features each
n_points = 500000
n_features = 5
n_clusters = 5  # Reduced from 8 to 5

# Create data with more distinct clusters
np.random.seed(42)
# Create well-separated cluster centers
centers = np.random.uniform(-10, 10, size=(n_clusters, n_features))

# Generate points around centers with varying variance
points_per_cluster = n_points // n_clusters
data = []
for i, center in enumerate(centers):
    # Vary the spread for each cluster
    variance = 0.2 + (i * 0.1)  # Different spread for each cluster
    cluster_points = center + np.random.randn(points_per_cluster, n_features) * variance
    data.append(cluster_points)

data = np.vstack(data)

# Save in the required format (tab-separated, with ID column)
with open('kmeans_data.txt', 'w') as f:
    for i, point in enumerate(data):
        f.write(f"{i}\t" + "\t".join(f"{x:.6f}" for x in point) + "\n")

print(f"Generated {n_points} points with {n_features} features")
print(f"Created {n_clusters} clusters with varying spreads")

# Verify the file
with open('kmeans_data.txt', 'r') as f:
    first_line = f.readline().strip().split('\t')
    print(f"First line has {len(first_line)} columns (1 ID + {len(first_line)-1} features)")
