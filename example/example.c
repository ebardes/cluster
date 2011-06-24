/* This is an example program that makes use of the C Clustering Library. */

/* ========================================================================= */

#include <stdio.h>
#include <stdlib.h>  /* The standard C libraries */

#include "cluster.h" /* The C Clustering Library */

/* ========================================================================= */

void show_data(int nrows, int ncols, double** data, int** mask)
/* Print the data matrix */
{ int i, j;
  printf("============== The gene expression data matrix ================\n");
  for (j = 0; j < ncols; j++) printf("\tCol %d", j);
  printf ("\n");
  for (i = 0; i < nrows; i++)
  { printf("Row %d", i);
    for (j = 0; j < ncols; j++)
    { if (mask[i][j]) printf("\t%5.2g",data[i][j]);
      else printf("\t"); /* mask[i][j]==0, so this data point is missing */
    }
    printf("\n");
  }
  printf("\n");
  return;
}

/* ========================================================================= */

void example_mean_median(int nrows, int ncols, double** data, int** mask)
{ int i, j;
  double* temp = malloc(ncols*sizeof(double));
  printf("============== Calculating the mean and median ================\n");
  for (i = 0; i < nrows; i++)
  { double meanvalue;
    double medianvalue;
    int ndata = 0;
    for (j = 0; j < ncols; j++)
    { if (mask[i][j])
      { temp[ndata] = data[i][j];
        ndata++;
      }
    }
    meanvalue = mean(ndata, temp);
    medianvalue = median(ndata, temp);
    printf("row %2d:\t", i);
    printf("mean = %7.3f\t", meanvalue);
    printf("median = %7.3f\n", medianvalue);
    /* Note that the median routine changes the order of the elements in
     * the array temp. */
  }
  printf("\n");
  free(temp);
}

/* ========================================================================= */

double** example_distance_gene(int nrows, int ncols, double** data, int** mask)
/* Calculate the distance matrix between genes using the Euclidean distance. */
{ int i, j;
  double** distMatrix;
  double* weight = malloc(ncols*sizeof(double));
  printf("============ Euclidean distance matrix between genes ============\n");
  for (i = 0; i < ncols; i++) weight[i] = 1.0;
  distMatrix = distancematrix(nrows, ncols, data, mask, weight, 'e', 0);
  if (!distMatrix)
  { printf ("Insufficient memory to store the distance matrix\n");
    free(weight);
    return NULL;
  }
  printf("   Gene:");
  for(i=0; i<nrows-1; i++) printf("%6d", i);
  printf("\n");
  for(i=0; i<nrows; i++)
  { printf("Gene %2d:",i);
    for(j=0; j<i; j++) printf(" %5.2f",distMatrix[i][j]);
    printf("\n");
  }
  printf("\n");
  free(weight);
  return distMatrix;
}

/* ========================================================================= */

void example_distance_array(int nrows, int ncols, double** data, int** mask)
/* Calculate the distance matrix between microarrays using the Spearman rank
 * correlation. */
{ int i, j;
  double** distMatrix;
  double* weight = malloc(nrows*sizeof(double));
  printf("========== Spearman distance matrix between microarrays =========\n");
  for (i = 0; i < nrows; i++) weight[i] = 1.0;
  distMatrix = distancematrix(nrows, ncols, data, mask, weight, 's', 1);
  if (!distMatrix)
  { printf ("Insufficient memory to store the distance matrix\n");
    free(weight);
    return;
  }
  printf("   Microarray:");
  for(i=0; i<ncols-1; i++) printf("%9d", i);
  printf("\n");
  for(i=0; i<ncols; ++i)
  { printf("Microarray %2d: ",i);
    for(j=0; j<i; ++j) printf(" %f",distMatrix[i][j]);
    printf("\n");
  }
  printf("\n");
  free(weight);
  for(i = 0; i < ncols; i++) free(distMatrix[i]);
  free(distMatrix);
  return;
}

/* ========================================================================= */

void example_hierarchical(int nrows, int ncols, double** data, int** mask,
			  double** distmatrix)
/* Perform hierarchical clustering on genes */
{ int i;
  const int nnodes = nrows-1;
  double* weight = malloc(ncols*sizeof(double));
  int* clusterid;
  Node* tree;
  for (i = 0; i < ncols; i++) weight[i] = 1.0;
  printf("\n");
  printf("================ Pairwise single linkage clustering ============\n");
  /* Since we have the distance matrix here, we may as well use it. */
  tree = treecluster(nrows, ncols, 0, 0, 0, 0, 'e', 's', distmatrix);
  /* The distance matrix was modified by treecluster, so we cannot use it any
   * more. But we still need to deallocate it here.
   * The first row of distmatrix is a single null pointer; no need to free it.
   */
  for (i = 1; i < nrows; i++) free(distmatrix[i]);
  free(distmatrix);
  if (!tree)
  { /* Indication that the treecluster routine failed */
    printf ("treecluster routine failed due to insufficient memory\n");
    free(weight);
    return;
  }
  printf("Node     Item 1   Item 2    Distance\n");
  for(i=0; i<nnodes; i++)
    printf("%3d:%9d%9d      %g\n",
           -i-1, tree[i].left, tree[i].right, tree[i].distance);
  printf("\n");
  free(tree);
  printf("================ Pairwise maximum linkage clustering ============\n");
  tree = treecluster(nrows, ncols, data, mask, weight, 0, 'e', 'm', 0);
  /* Here, we let treecluster calculate the distance matrix for us. In that
   * case, the treecluster routine may fail due to insufficient memory to store
   * the distance matrix. For the small data sets in this example, that is
   * unlikely to occur though. Let's check for it anyway:
   */
  if (!tree)
  { /* Indication that the treecluster routine failed */
    printf ("treecluster routine failed due to insufficient memory\n");
    free(weight);
    return;
  }
  printf("Node     Item 1   Item 2    Distance\n");
  for(i=0; i<nnodes; i++)
    printf("%3d:%9d%9d      %g\n",
           -i-1, tree[i].left, tree[i].right, tree[i].distance);
  printf("\n");
  free(tree);
  printf("================ Pairwise average linkage clustering ============\n");
  tree = treecluster(nrows, ncols, data, mask, weight, 0, 'e', 'a', 0); 
  if (!tree)
  { /* Indication that the treecluster routine failed */
    printf ("treecluster routine failed due to insufficient memory\n");
    free(weight);
    return;
  }
  printf("Node     Item 1   Item 2    Distance\n");
  for(i=0; i<nnodes; i++)
    printf("%3d:%9d%9d      %g\n",
           -i-1, tree[i].left, tree[i].right, tree[i].distance);
  printf("\n");
  free(tree);
  printf("================ Pairwise centroid linkage clustering ===========\n");
  tree = treecluster(nrows, ncols, data, mask, weight, 0, 'e', 'c', 0); 
  if (!tree)
  { /* Indication that the treecluster routine failed */
    printf ("treecluster routine failed due to insufficient memory\n");
    free(weight);
    return;
  }
  printf("Node     Item 1   Item 2    Distance\n");
  for(i=0; i<nnodes; i++)
    printf("%3d:%9d%9d      %g\n",
           -i-1, tree[i].left, tree[i].right, tree[i].distance);
  printf("\n");
  printf("=============== Cutting a hierarchical clustering tree ==========\n");
  clusterid = malloc(nrows*sizeof(int));
  cuttree (nrows, tree, 3, clusterid);
  for(i=0; i<nrows; i++)
    printf("Gene %2d: cluster %2d\n", i, clusterid[i]);
  printf("\n");
  free(tree);
  free(clusterid);
  free(weight);
  return;
}

/* ========================================================================= */

void example_kmeans(int nrows, int ncols, double** data, int** mask)
/* Perform k-means clustering on genes */
{ int i, j;
  const int nclusters = 3;
  const int transpose = 0;
  const char dist = 'e';
  const char method = 'a';
  int npass = 1;
  int ifound = 0;
  double error;
  double distance;
  int** index;
  int* count;
  double* weight = malloc(ncols*sizeof(double));
  int* clusterid = malloc(nrows*sizeof(int));
  double** cdata = malloc(nclusters*sizeof(double*));
  int** cmask = malloc(nclusters*sizeof(int*));
  for (i = 0; i < nclusters; i++)
  { cdata[i] = malloc(ncols*sizeof(double));
    cmask[i] = malloc(ncols*sizeof(int));
  }
  for (i = 0; i < ncols; i++) weight[i] = 1.0;
  printf("======================== k-means clustering ====================\n");
  printf("\n");
  printf("----- one pass of the EM algorithm (results may change)\n");
  kcluster(nclusters,nrows,ncols,data,mask,weight,transpose,npass,method,dist, 
    clusterid, &error, &ifound);
  printf ("Solution found %d times; within-cluster sum of distances is %f\n",
    ifound, error);
  printf ("Cluster assignments:\n");
  for (i = 0; i < nrows; i++)
    printf ("Gene %d: cluster %d\n", i, clusterid[i]);

  printf ("\n");
  printf("----- 1000 passes of the EM algorithm (result should not change)\n");
  npass = 1000;
  kcluster(nclusters,nrows,ncols,data,mask,weight,transpose,npass,method,dist, 
    clusterid, &error, &ifound);
  printf ("Solution found %d times; ", ifound);
  printf ("within-cluster sum of distances is %f\n", error);
  printf ("Cluster assignments:\n");
  for (i = 0; i < nrows; i++)
    printf ("Gene %d: cluster %d\n", i, clusterid[i]);
  printf ("\n");
  printf ("------- Distance between clusters:\n");
  index = malloc(nclusters*sizeof(int*));
  count = malloc(nclusters*sizeof(int));
  for (i = 0; i < nclusters; i++) count[i] = 0;
  for (i = 0; i < nrows; i++) count[clusterid[i]]++;
  for (i = 0; i < nclusters; i++) index[i] = malloc(count[i]*sizeof(int));
  for (i = 0; i < nclusters; i++) count[i] = 0;
  for (i = 0; i < nrows; i++)
  { int id = clusterid[i];
    index[id][count[id]] = i;
    count[id]++;
  }  
  distance =
    clusterdistance(nrows, ncols, data, mask, weight, count[0], count[1],
		    index[0], index[1], 'e', 'a', 0); 
  printf("Distance between 0 and 1: %7.3f\n", distance);
  distance =
    clusterdistance(nrows, ncols, data, mask, weight, count[0], count[2],
		    index[0], index[2], 'e', 'a', 0); 
  printf("Distance between 0 and 2: %7.3f\n", distance);
  distance =
    clusterdistance(nrows, ncols, data, mask, weight, count[1], count[2],
		    index[1], index[2], 'e', 'a', 0); 
  printf("Distance between 1 and 2: %7.3f\n", distance);

  printf ("\n");
  printf ("------- Cluster centroids:\n");
  getclustercentroids(nclusters, nrows, ncols, data, mask, clusterid,
                      cdata, cmask, 0, 'a');
  printf("   Microarray:");
  for(i=0; i<ncols; i++) printf("\t%7d", i);
  printf("\n");
  for (i = 0; i < nclusters; i++)
  { printf("Cluster %2d:", i);
    for (j = 0; j < ncols; j++) printf("\t%7.3f", cdata[i][j]);
    printf("\n");
  }
  printf("\n");
  for (i = 0; i < nclusters; i++) free(index[i]);
  free(index);
  free(count);

  for (i = 0; i < nclusters; i++)
  { free(cdata[i]);
    free(cmask[i]);
  }
  free(cdata);
  free(cmask);
  free(clusterid);
  free(weight);
  return;
}

/* ========================================================================= */

void example_som(int nrows, int ncols, double** data, int** mask)
/* Calculate a self-organizing map, applied to genes */
{ int i, j, k;
  const int nxgrid = 2;
  const int nygrid = 2; /* Rectangular grid 2x2 */
  const double inittau = 0.02; /* Initial value of the neighborhood function */
  const int niter = 1000; /* Number of iterations */
  const char dist = 'c'; /* Pearson correlation */
  double* weight = malloc(ncols*sizeof(double));
  int (*clusterid)[2] = malloc(nrows*sizeof(int[2])); 
  double*** celldata = malloc(nxgrid*sizeof(double**));
  for (i = 0; i < nxgrid; i++)
  { celldata[i] = malloc(nygrid*sizeof(double*));
    for (j = 0; j < nygrid; j++)
      celldata[i][j] = malloc(ncols*sizeof(double));
  }
  for (i = 0; i < ncols; i++) weight[i] = 1.0;
  printf("======================= Self-Organizing Map ===================\n");
  printf("(results may change on every run)\n");
  somcluster(nrows, ncols, data, mask, weight, 0, nxgrid, nygrid, inittau, 
	     niter, dist, celldata, clusterid);
  printf("Cluster assignments:\n");
  for(i=0; i<nrows; i++)
    printf("Gene %2d: %2d %2d\n",i,clusterid[i][0],clusterid[i][1]);
  printf("Cluster centroids:\n");
  printf("\t");
  for (j = 0; j < ncols; j++) printf("\tCol %d", j);
  printf ("\n");
  for (i = 0; i < nxgrid; i++)
  { for (j = 0; j < nygrid; j++)
    { printf("Cell (%d,%d):", i, j);
      for (k = 0; k < ncols; k++) printf("\t%5.2g",celldata[i][j][k]);
      printf("\n");
    }
  }
  printf("\n");
  /* Deallocate memory */
  for (i = 0; i < nxgrid; i++)
  { for (j = 0; j < nygrid; j++) free(celldata[i][j]);
    free(celldata[i]);
  }
  free(celldata);
  free(weight);
  free(clusterid);
  return;
}

/* ========================================================================= */

int main(void)
{ int i;
  const int nrows = 13;
  const int ncols = 4;
  double** data = malloc(nrows*sizeof(double*) );
  int** mask = malloc(nrows*sizeof(int*));
  double** distmatrix;

  for (i = 0; i < nrows; i++)
  { data[i] = malloc(ncols*sizeof(double));
    mask[i] = malloc(ncols*sizeof(int));
  }

  /* Test data, roughly distributed in 0-5, 6-8, 9-12 */
  data[ 0][ 0]=0.1; data[ 0][ 1]=0.0; data[ 0][ 2]=9.6; data[ 0][ 3] = 5.6;
  data[ 1][ 0]=1.4; data[ 1][ 1]=1.3; data[ 1][ 2]=0.0; data[ 1][ 3] = 3.8;
  data[ 2][ 0]=1.2; data[ 2][ 1]=2.5; data[ 2][ 2]=0.0; data[ 2][ 3] = 4.8;
  data[ 3][ 0]=2.3; data[ 3][ 1]=1.5; data[ 3][ 2]=9.2; data[ 3][ 3] = 4.3;
  data[ 4][ 0]=1.7; data[ 4][ 1]=0.7; data[ 4][ 2]=9.6; data[ 4][ 3] = 3.4;
  data[ 5][ 0]=0.0; data[ 5][ 1]=3.9; data[ 5][ 2]=9.8; data[ 5][ 3] = 5.1;
  data[ 6][ 0]=6.7; data[ 6][ 1]=3.9; data[ 6][ 2]=5.5; data[ 6][ 3] = 4.8;
  data[ 7][ 0]=0.0; data[ 7][ 1]=6.3; data[ 7][ 2]=5.7; data[ 7][ 3] = 4.3;
  data[ 8][ 0]=5.7; data[ 8][ 1]=6.9; data[ 8][ 2]=5.6; data[ 8][ 3] = 4.3;
  data[ 9][ 0]=0.0; data[ 9][ 1]=2.2; data[ 9][ 2]=5.4; data[ 9][ 3] = 0.0;
  data[10][ 0]=3.8; data[10][ 1]=3.5; data[10][ 2]=5.5; data[10][ 3] = 9.6;
  data[11][ 0]=0.0; data[11][ 1]=2.3; data[11][ 2]=3.6; data[11][ 3] = 8.5;
  data[12][ 0]=4.1; data[12][ 1]=4.5; data[12][ 2]=5.8; data[12][ 3] = 7.6;

  /* Some data are actually missing */
  mask[ 0][ 0]=1; mask[ 0][ 1]=1; mask[ 0][ 2]=1; mask[ 0][ 3] = 1;
  mask[ 1][ 0]=1; mask[ 1][ 1]=1; mask[ 1][ 2]=0; mask[ 1][ 3] = 1;
  mask[ 2][ 0]=1; mask[ 2][ 1]=1; mask[ 2][ 2]=0; mask[ 2][ 3] = 1;
  mask[ 3][ 0]=1; mask[ 3][ 1]=1; mask[ 3][ 2]=1; mask[ 3][ 3] = 1;
  mask[ 4][ 0]=1; mask[ 4][ 1]=1; mask[ 4][ 2]=1; mask[ 4][ 3] = 1;
  mask[ 5][ 0]=0; mask[ 5][ 1]=1; mask[ 5][ 2]=1; mask[ 5][ 3] = 1;
  mask[ 6][ 0]=1; mask[ 6][ 1]=1; mask[ 6][ 2]=1; mask[ 6][ 3] = 1;
  mask[ 7][ 0]=0; mask[ 7][ 1]=1; mask[ 7][ 2]=1; mask[ 7][ 3] = 1;
  mask[ 8][ 0]=1; mask[ 8][ 1]=1; mask[ 8][ 2]=1; mask[ 8][ 3] = 1;
  mask[ 9][ 0]=1; mask[ 9][ 1]=1; mask[ 9][ 2]=1; mask[ 9][ 3] = 0;
  mask[10][ 0]=1; mask[10][ 1]=1; mask[10][ 2]=1; mask[10][ 3] = 1;
  mask[11][ 0]=0; mask[11][ 1]=1; mask[11][ 2]=1; mask[11][ 3] = 1;
  mask[12][ 0]=1; mask[12][ 1]=1; mask[12][ 2]=1; mask[12][ 3] = 1;

  show_data(nrows, ncols, data, mask);
  example_mean_median(nrows, ncols, data, mask);
  distmatrix = example_distance_gene(nrows, ncols, data, mask);
  if (distmatrix) example_hierarchical(nrows, ncols, data, mask, distmatrix);
  example_distance_array(nrows, ncols, data, mask);
  example_kmeans(nrows, ncols, data, mask);
  example_som(nrows, ncols, data, mask);

  return 0;
}


