#import "Controller.h"
#import "data.h"

#import <unistd.h> /* contains dup */

typedef struct
/* So tired of Objective-C ... let's make a good old struct instead. */
{   NSFileHandle* handle;
    FILE* pointer;
}   FileHandle;

static BOOL OpenFile(FileHandle* file, NSString* path, char mode[])
{
    file->pointer = NULL; /* Initialization */
    switch (mode[0])
    { case 'w':
        file->handle = [NSFileHandle fileHandleForWritingAtPath: path];
        break;
      case 'r':
        file->handle = [NSFileHandle fileHandleForReadingAtPath: path];
        break;
      default: return FALSE;
    }
    if (!file->handle) return FALSE;
    int fd = [file->handle fileDescriptor];
    /* Duplicate the file descriptor so that we can call fclose on pointer
     * and release on handle. Stupid Cocoa makes life difficult. */
    file->pointer = fdopen(dup(fd), mode);
    if (!file->pointer) {
        [file->handle closeFile];
        return FALSE;
    }
    [file->handle retain];
    return TRUE;
}

static void CloseFile(FileHandle file)
{
    fclose(file.pointer);
    [file.handle release];
}

@implementation Controller
- (void)UpdateInfo
{
    const int rows = GetRows();
    const int columns = GetColumns();
    [Rows setIntValue: rows];
    [Columns setIntValue: columns];
    int dim = 1 + (int)pow(rows,0.25);
    [SOMGeneXDim setIntValue: dim];
    [SOMGeneYDim setIntValue: dim];
    dim = 1 + (int)pow(columns,0.25);
    [SOMArrayXDim setIntValue: dim];
    [SOMArrayYDim setIntValue: dim];
}

- (void)FilterReset
{
    [filterresult setStringValue: @""];
    [filteraccept setEnabled: FALSE];
}

- (void)InitDir
{
   if(directory) return;
   directory = NSHomeDirectory();
   [directory retain];
}

- (void)SaveDir:(NSString*)filename
{
    if(directory) [directory release];
    directory = [filename stringByDeletingLastPathComponent];
    [directory retain];
}

- (char)GetMetric:(NSComboBox*)metricbox
{
    int index = [metricbox indexOfSelectedItem];
    switch (index) {
  	case 0: return 'u'; break; // Uncentered correlation
        case 1: return 'c'; break; // Centered correlation
        case 2: return 'x'; break; // Absolute uncentered correlation
        case 3: return 'a'; break; // Absolute centered correlation
        case 4: return 's'; break; // Spearman rank correlation
        case 5: return 'k'; break; // Kendall's tau
        case 6: return 'e'; break; // Euclidean distance
        case 7: return 'b'; break; // City-block distance
        // The code will never get here.
        default: return 'e'; //Euclidean distance is default.
    }
}

- (IBAction)FileOpen:(id)sender
{
    int result;
    NSOpenPanel *oPanel = [NSOpenPanel openPanel];

    [statusbar setStringValue: @"Opening file"];
    
    [self InitDir];

    result = [oPanel runModalForDirectory:directory file:nil types:nil];
    if (result == NSOKButton) {
        NSString *aFile = [oPanel filename];
        FileHandle inputfile;
        [self SaveDir: aFile];
        if (!OpenFile(&inputfile, aFile, "rt")) {
            NSRunCriticalAlertPanel(@"Error opening file",
                                    @"Error",
                                    @"OK",
                                    nil,
                                    nil);
            return;
        }
        char* result = Load(inputfile.pointer);
        CloseFile(inputfile);
        [FileMemo setStringValue: @""];
        [JobName setStringValue: @""];
        [Rows setStringValue: @""];
        [Columns setStringValue: @""];
        [self FilterReset];
        if (!result)
        {
            NSRunCriticalAlertPanel(@"Error reading file",
                                    @"Insufficient memory",
                                    @"OK",
                                    nil,
                                    nil);
            [statusbar setStringValue:
                [@"Error reading file " stringByAppendingString: aFile]];
            return;
        }
        else if (strcmp(result, "ok")!=0)
        {
            NSRunCriticalAlertPanel(@"Error in data file",
                                    [NSString stringWithCString: result],
                                    @"OK",
                                    nil,
                                    nil);
            free(result);
            [statusbar setStringValue:
                [@"Error reading file " stringByAppendingString: aFile]];
            return;
        }
        [statusbar setStringValue: @"Done loading data"];
        /* Extract job name from file name */
        [JobName setStringValue: [[aFile lastPathComponent] stringByDeletingPathExtension]];
        [FileMemo setStringValue: aFile];
        [self UpdateInfo];
    } else {
        [statusbar setStringValue: @"Cancelled"];
    }
}

- (IBAction)FileSave:(id)sender
{
    NSSavePanel *sPanel;
    int result;

    /* create or get the shared instance of NSSavePanel */
    sPanel = [NSSavePanel savePanel];
    
    [statusbar setStringValue: @"Saving data to file"];
    [statusbar display];
    
    [self InitDir];

    /* display the NSSavePanel */
    result = [sPanel runModalForDirectory:directory file:[[JobName stringValue] stringByAppendingPathExtension:@"txt"]];

    /* if successful, save file under designated name */
    if (result == NSOKButton) {
        NSString *aFile = [sPanel filename];
        [[NSFileManager defaultManager] createFileAtPath: aFile
                                                contents: nil
                                              attributes: nil];
        FileHandle outputfile;
        [self SaveDir: aFile];
        if (!OpenFile(&outputfile, aFile, "wt")) {
            [statusbar setStringValue: @"Error: Unable to open the output file"];
            return;
        }
        result = Save(outputfile.pointer, 0, 0);
        CloseFile(outputfile);
        if (result)
            [statusbar setStringValue: @"Finished saving file"];
        else
        {
            NSRunCriticalAlertPanel(@"Error saving file",
                                    @"Insufficient memory",
                                    @"OK",
                                    nil,
                                    nil);
            [statusbar setStringValue: @"Error saving to file"];
        }
    } else {
        [statusbar setStringValue: @"Cancelled"];
    }
}

- (IBAction)ShowHelpManual:(id)sender
{
    [[NSWorkspace sharedWorkspace] openFile: @"/Applications/Cluster.app/Contents/Resources/cluster3.pdf"];
}

- (IBAction)ShowHelpDownload:(id)sender
{
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: @"http://bonsai.ims.u-tokyo.ac.jp/~mdehoon/software/cluster/manual"]];
}

- (IBAction)FilterApply:(id)sender
{
    [self FilterReset];
    const int rows = GetRows();
    /* Store results in boolean use */
    free(use);
    use = malloc(rows*sizeof(int));
    useRows = 0;

    const BOOL bStd = [FilterStdXB state];
    const BOOL bPercent = [FilterPercentXB state];
    const BOOL bAbsVal = [FilterObservationXB state];
    const BOOL bMaxMin = [FilterMaxMinXB state];

    /* Read information from the edit boxes */
    const double absVal = [[FilterObservationValue stringValue] doubleValue];
    const double percent = [[FilterPercent stringValue] doubleValue];
    const double std = [[FilterStd stringValue] doubleValue];
    const int numberAbs = [FilterNumber intValue];
    const double maxmin = [[FilterMaxMin stringValue] doubleValue];
    /* Note: Applying doubleValue directly on a NSTextField causes the text to
     * be interpreted with following the current localization. In particular,
     * with European localization settings, numbers such as 0.02 are
     * misinterpreted, as they should be 0,02 according to that localization.
     * However, applying doubleValue to an NSString instance always expects the
     * US number format. */

    int Row;
    for (Row = 0; Row < rows; Row++) {
        [statusbar setStringValue: [NSString stringWithFormat: @"Assessing filters for gene %d", Row]];
        use[Row] = FilterRow(Row,bStd,bPercent,bAbsVal,bMaxMin,absVal,percent,std,numberAbs,maxmin);
        /* Count how many passed */
        if (use[Row]) useRows++;
    }
    /* Tell user how many rows passed */
    [filterresult setStringValue: [NSString stringWithFormat: @"%d passed out of %d", useRows, rows]];
    [filteraccept setEnabled: TRUE];
    [statusbar setStringValue: @"Done Analyzing Filters"];
}

- (IBAction)FilterAccept:(id)sender
{
    int ok;
    [filteraccept setEnabled: FALSE];
    ok = SelectSubset(useRows, use);
    if (!ok) {
        NSRunCriticalAlertPanel(@"Insufficient memory",
                                @"Memory allocation error",
                                @"OK",
                                nil,
                                nil);
        [statusbar setStringValue: @"Error accepting filter"];
    }
    else [self UpdateInfo];
}

- (IBAction)AdjustApply:(id)sender
{
    int ok;
    [statusbar setStringValue: @"Adjusting data"];
    [statusbar display];
    const BOOL bLogTransform = [AdjustLogXB state];
    if (bLogTransform) LogTransform();
    const int GeneCenter = [AdjustCenterGenesXB state];
    const int GeneMeanCenter = GeneCenter && [AdjustMeanGenes state];
    const int GeneMedianCenter = GeneCenter && [AdjustMedianGenes state];
    const int GeneNormalize = [AdjustNormalizeGenes state];
    ok = AdjustGenes(GeneMeanCenter, GeneMedianCenter, GeneNormalize);
    if (!ok) {
        NSRunCriticalAlertPanel(@"Insufficient memory",
                                @"Memory allocation error",
                                @"OK",
                                nil,
                                nil);
        [statusbar setStringValue: @"Error adjusting genes"];
        return;
    }
    const int ArrayCenter = [AdjustCenterArraysXB state];
    const int ArrayMeanCenter = ArrayCenter && [AdjustMeanArrays state];
    const int ArrayMedianCenter = ArrayCenter && [AdjustMedianArrays state];
    const int ArrayNormalize = [AdjustNormalizeArrays state];
    ok = AdjustArrays(ArrayMeanCenter, ArrayMedianCenter, ArrayNormalize);
    if (!ok) {
        NSRunCriticalAlertPanel(@"Insufficient memory",
                                @"Memory allocation error",
                                @"OK",
                                nil,
                                nil);
        [statusbar setStringValue: @"Error adjusting arrays"];
        return;
    }
    [statusbar setStringValue: @"Done adjusting data"];
}

- (IBAction)AdjustCenterGenesXBChanged:(id)sender
{
    if ([AdjustCenterGenesXB state]) {
        [AdjustMeanGenes setEnabled: true];
        [AdjustMedianGenes setEnabled: true];
    } else {
        [AdjustMeanGenes setEnabled: false];
        [AdjustMedianGenes setEnabled: false];
    }
}

- (IBAction)AdjustCenterArraysXBChanged:(id)sender
{
    if ([AdjustCenterArraysXB state]) {
        [AdjustMeanArrays setEnabled: true];
        [AdjustMedianArrays setEnabled: true];
    } else {
        [AdjustMeanArrays setEnabled: false];
        [AdjustMedianArrays setEnabled: false];
    }
}

- (IBAction)HierarchicalGeneWeightXBChanged:(id)sender
{
    if ([HierarchicalGeneWeightXB state]) {
        [HierarchicalArrays addSubview: HierarchicalArrayWeight];
    } else {
        [HierarchicalArrayWeight retain];
        [HierarchicalArrayWeight removeFromSuperview];
    }
}

- (IBAction)HierarchicalArrayWeightXBChanged:(id)sender
{
    if ([HierarchicalArrayWeightXB state]) {
        [HierarchicalGenes addSubview: HierarchicalGeneWeight];
    } else {
        [HierarchicalGeneWeight retain];
        [HierarchicalGeneWeight removeFromSuperview];
    }
}

- (IBAction)HierarchicalCentroid:(id)sender
{
    NSString* method = @"c";
/* Multithreading is not yet implemented, but this is how it would look like
    [NSThread detachNewThreadSelector: @selector(HierarchicalExecute:)
                             toTarget: self
                           withObject: method];
*/
    [self HierarchicalExecute: method];
}

- (IBAction)HierarchicalSingle:(id)sender
{
    NSString* method = @"s";
/* Multithreading is not yet implemented, but this is how it would look like
    [NSThread detachNewThreadSelector: @selector(HierarchicalExecute:)
                             toTarget: self
                           withObject: method];
*/
    [self HierarchicalExecute: method];
}

- (IBAction)HierarchicalComplete:(id)sender
{
    NSString* method = @"m";
/* Multithreading is not yet implemented, but this is how it would look like
    [NSThread detachNewThreadSelector: @selector(HierarchicalExecute:)
                             toTarget: self
                           withObject: method];
*/
    [self HierarchicalExecute: method];
}

- (IBAction)HierarchicalAverage:(id)sender
{
    NSString* method = @"a";
/* Multithreading is not yet implemented, but this is how it would look like
    [NSThread detachNewThreadSelector: @selector(HierarchicalExecute:)
                             toTarget: self
                           withObject: method];
*/
    [self HierarchicalExecute: method];
}

- (void)HierarchicalExecute:(NSString*)method_string
{
    int ok;
    const int rows = GetRows();
    const int columns = GetColumns();
    const char method = *([method_string cString]);
    if (rows==0 || columns==0) {
        [statusbar setStringValue: @"No data available"];
        return;
    }

    // Find out what we need to do here
    const BOOL ClusterGenes = [HierarchicalGeneXB state];
    const BOOL ClusterArrays = [HierarchicalArrayXB state];
    if(!ClusterGenes && !ClusterArrays) return;
    // Nothing to do here

    const BOOL bCalculateGeneWeights = [HierarchicalGeneWeightXB state];
    const BOOL bCalculateArrayWeights = [HierarchicalArrayWeightXB state];

/* For a multithreaded application, this routine would need its own
 * autorelease pool.
    NSAutoreleasePool* threadPool = [[NSAutoreleasePool alloc] init];
*/

    NSString* base = [[FileMemo stringValue] stringByDeletingLastPathComponent];
    NSString* jobname = [base stringByAppendingPathComponent: [JobName stringValue]];

    // Find out which metrics to use
    const char genemetric = [self GetMetric: HierarchicalGeneMetric];
    const char arraymetric = [self GetMetric: HierarchicalArrayMetric];

    if (bCalculateGeneWeights || bCalculateArrayWeights) {
    	const char* error;
        double gene_cutoff = 0.0;
        double gene_exponent = 0.0;
        double array_cutoff = 0.0;
        double array_exponent = 0.0;

        [statusbar setStringValue: @"Calculating weights"];
        [statusbar display];

        if (bCalculateGeneWeights) {
          array_cutoff = [[HierarchicalArrayCutoff stringValue] doubleValue];
          array_exponent = [[HierarchicalArrayExp stringValue] doubleValue];
        }
        if (bCalculateArrayWeights) {
          gene_cutoff = [[HierarchicalGeneCutoff stringValue] doubleValue];
          gene_exponent = [[HierarchicalGeneExp stringValue] doubleValue];
        }
        error = CalculateWeights(gene_cutoff, gene_exponent, genemetric,
                                 array_cutoff, array_exponent, arraymetric);
        if (error) {
            [statusbar setStringValue: [NSString stringWithCString: error]];
            return;
        }
    }

    switch(method) {
      case 'c':
        [statusbar setStringValue: @"Performing centroid linkage hierarchical clustering"];
        break;
      case 's':
        [statusbar setStringValue: @"Performing single linkage hierarchical clustering"];
        break;
      case 'm':
        [statusbar setStringValue: @"Performing complete linkage hierarchical clustering"];
        break;
      case 'a':
        [statusbar setStringValue: @"Performing average linkage hierarchical clustering"];
        break;
    }
    [statusbar display];

    if (ClusterGenes) {
        NSString* filename = [jobname stringByAppendingPathExtension: @"gtr"];
        [[NSFileManager defaultManager] createFileAtPath: filename
                                                contents: nil
                                              attributes: nil];
        FileHandle outputfile;
        if (!OpenFile(&outputfile, filename, "wt")) {
            [statusbar setStringValue: @"Error: Unable to open the output file"];
            return;
        }
        ok = HierarchicalCluster(outputfile.pointer, genemetric, 0, method);
        CloseFile(outputfile);
        if (!ok) {
            [statusbar setStringValue: @"Error: Insufficient memory"];
            return;
        }
    }

    if (ClusterArrays) {
        NSString* filename = [jobname stringByAppendingPathExtension: @"atr"];
        [[NSFileManager defaultManager] createFileAtPath: filename
                                                contents: nil
                                              attributes: nil];
        FileHandle outputfile;
        if (!OpenFile(&outputfile, filename, "wt")) {
            [statusbar setStringValue: @"Error: Unable to open the output file"];
            return;
        }
        ok = HierarchicalCluster(outputfile.pointer, arraymetric, 1, method);
        CloseFile(outputfile);
        if (!ok) {
            [statusbar setStringValue: @"Error: Insufficient memory"];
            return;
        }
    }

    [statusbar setStringValue: @"Saving the clustering result"];
    [statusbar display];

    // Now make output .cdt file
    NSString* filename = [jobname stringByAppendingPathExtension: @"cdt"];
    [[NSFileManager defaultManager] createFileAtPath: filename
                                            contents: nil
                                          attributes: nil];
    FileHandle outputfile;
    if (!OpenFile(&outputfile, filename, "wt")) {
        [statusbar setStringValue: @"Error: Unable to open the output file"];
        return;
    }
    ok = Save(outputfile.pointer, ClusterGenes, ClusterArrays);
    CloseFile(outputfile);
    if (!ok)
    {
        NSRunCriticalAlertPanel(@"Error saving file",
                                @"Insufficient memory",
                                @"OK",
                                nil,
                                nil);
        [statusbar setStringValue: @"Error saving to file"];
    }
    else
        [statusbar setStringValue: @"Done Clustering"];
/* Release this thread's autorelease pool here in a multithreaded application
    [threadPool release];
 */
}

- (IBAction)KMeansExecute:(id)sender
{
    int ok;
    const int rows = GetRows();
    const int columns = GetColumns();
    if (rows==0 || columns==0) {
        [statusbar setStringValue: @"No data available"];
        return;
    }
    
    NSString* base = [[FileMemo stringValue] stringByDeletingLastPathComponent];
    NSString* jobname = [base stringByAppendingPathComponent: [JobName stringValue]];

    const BOOL ClusterGenes = [KMeansGeneXB state];
    const BOOL ClusterArrays = [KMeansArrayXB state];
    if (!ClusterGenes && !ClusterArrays) return; // Nothing to do

    [statusbar setStringValue: @"Executing k-means clustering"];
    [statusbar display];

    int kGenes = 0;
    int kArrays = 0;

    if (ClusterGenes) {
        kGenes = [KMeansGeneK intValue];
        if (kGenes==0) {
            [statusbar setStringValue: @"Choose a nonzero number of clusters"];
            return;
        }
        if (rows < kGenes) {
            [statusbar setStringValue: @"More clusters than genes available"];
            return;
        }

        const char method = [KMeansGeneMean state] ? 'a' : 'm';
        // 'a' is average, 'm' is median

        const char dist = [self GetMetric: KMeansGeneMetric];
        int *NodeMap = malloc(rows*sizeof(int));
        const int nGeneTrials = [KMeansGeneRuns intValue];
        ok = 1;
        NSString* filename = nil;
        int ifound = GeneKCluster(kGenes, nGeneTrials, method, dist, NodeMap);
        if (ifound < 0) ok = 0;
        if (ok) {
            [statusbar setStringValue: [NSString stringWithFormat: @"Solution was found %d times", ifound]];
        
            filename = [jobname stringByAppendingFormat: @"_K_G%d.kgg", kGenes];
            [[NSFileManager defaultManager] createFileAtPath: filename
                                                    contents: nil
                                                  attributes: nil];
            FileHandle outputfile;
            if (!OpenFile(&outputfile, filename, "wt")) {
                [statusbar setStringValue: @"Error: Unable to open the output file"];
                free(NodeMap);
                return;
            }
            ok = SaveGeneKCluster(outputfile.pointer, kGenes, NodeMap);
            CloseFile(outputfile);
        }
        free(NodeMap);
        if (!ok) {
            NSRunCriticalAlertPanel(@"Insufficient memory",
                                    @"Memory allocation problem",
                                    @"OK",
                                    nil,
                                    nil);
            [statusbar setStringValue:
                [@"Error saving file " stringByAppendingString: filename]];
            return;
        }
    }
    
    if (ClusterArrays) {
        kArrays = [KMeansArrayK intValue];
        if (kArrays==0) {
            [statusbar setStringValue: @"Choose a nonzero number of clusters"];
            return;
        }
        if (columns < kArrays) {
            [statusbar setStringValue: @"More clusters than experiments available"];
            return;
        }

        const char method = [KMeansArrayMean state] ? 'a' : 'm';
        // 'a' is average, 'm' is median

        const char dist = [self GetMetric: KMeansArrayMetric];
        int *NodeMap = malloc(columns*sizeof(int));
        const int nArrayTrials = [KMeansArrayRuns intValue];

        ok = 1;
        NSString* filename = nil;
        int ifound = ArrayKCluster(kArrays, nArrayTrials, method, dist, NodeMap);
        if (ifound < 0) ok = 0;
        if (ok) {
            [statusbar setStringValue: [NSString stringWithFormat: @"Solution was found %d times", ifound]];
        
            filename = [jobname stringByAppendingFormat: @"_K_A%d.kag", kArrays];
            [[NSFileManager defaultManager] createFileAtPath: filename
                                                    contents: nil
                                                  attributes: nil];
            FileHandle outputfile;
            if (!OpenFile(&outputfile, filename, "wt")) {
                [statusbar setStringValue: @"Error: Unable to open the output file"];
                free(NodeMap);
                return;
            }
            ok = SaveArrayKCluster(outputfile.pointer, kArrays, NodeMap);
            CloseFile(outputfile);
        }
        free(NodeMap);
        if (!ok) {
            NSRunCriticalAlertPanel(@"Insufficient memory",
                                    @"Memory allocation problem",
                                    @"OK",
                                    nil,
                                    nil);
            [statusbar setStringValue:
                [@"Error saving file " stringByAppendingString: filename]];
            return;
        }
    }

    NSString* filename = 0;
    if (ClusterGenes && ClusterArrays)
        filename = [jobname stringByAppendingFormat: @"_K_G%d_A%d", kGenes, kArrays];
    else if (ClusterGenes)
        filename = [jobname stringByAppendingFormat: @"_K_G%d", kGenes];
    else if (ClusterArrays)
        filename = [jobname stringByAppendingFormat: @"_K_A%d", kArrays];
    filename = [filename stringByAppendingPathExtension: @"cdt"];
    [[NSFileManager defaultManager] createFileAtPath: filename
                                            contents: nil
                                          attributes: nil];
    FileHandle outputfile;
    if (!OpenFile(&outputfile, filename, "wt")) {
        [statusbar setStringValue: @"Error: Unable to open the output file"];
        return;
    }
    ok = Save(outputfile.pointer, 0, 0);
    CloseFile(outputfile);
    if (!ok)
    {
        NSRunCriticalAlertPanel(@"Error saving file",
                                @"Insufficient memory",
                                @"OK",
                                nil,
                                nil);
        [statusbar setStringValue: @"Error saving to file"];
    }
    else
        [statusbar setStringValue: @"Finished saving file"];
}

- (IBAction)SOMExecute:(id)sender
{
    const int rows = GetRows();
    const int columns = GetColumns();
    if (rows==0 || columns==0) {
        [statusbar setStringValue: @"No data available"];
        return;
    }
    
    const BOOL ClusterGenes = [SOMGeneXB state];
    const BOOL ClusterArrays = [SOMArrayXB state];
	
    if (!ClusterGenes && ! ClusterArrays) return;
    // Nothing to do here

    const int GeneXDim = ClusterGenes ? [SOMGeneXDim intValue] : 0;
    const int GeneYDim = ClusterGenes ? [SOMGeneYDim intValue] : 0;
    const int ArrayXDim = ClusterArrays ? [SOMArrayXDim intValue] : 0;
    const int ArrayYDim = ClusterArrays ? [SOMArrayYDim intValue] : 0;

    if((ClusterGenes && (GeneXDim==0 || GeneYDim==0)) ||
       (ClusterArrays && (ArrayXDim==0 || ArrayYDim==0)))
    {   [statusbar setStringValue: @"Error starting SOM: Check SOM dimensions"];
        return;
    }

    const int GeneIters = ClusterGenes ? [SOMGeneIters intValue] : 0;
    const double GeneTau = [[SOMGeneTau stringValue] doubleValue];
    const char GeneMetric = [self GetMetric: SOMGeneMetric];

    const int ArrayIters = ClusterArrays ? [SOMArrayIters intValue] : 0;
    const double ArrayTau = [[SOMArrayTau stringValue] doubleValue];
    const char ArrayMetric = [self GetMetric: SOMArrayMetric];

    FileHandle GeneFile;
    FileHandle ArrayFile;
    FileHandle DataFile;

    NSString* filename;
    NSString* base = [[FileMemo stringValue] stringByDeletingLastPathComponent];
    NSString* jobname = [base stringByAppendingPathComponent: [JobName stringValue]];

    [statusbar setStringValue: @"Calculating Self-Organizing Map"];
    [statusbar display];
    // To make sure statusbar gets updated before the calculation starts

    NSMutableString* basename = [NSMutableString stringWithCapacity: 0];
    [basename appendString: jobname];
    [basename appendString: @"_SOM"];
    if(ClusterGenes) [basename appendFormat: @"_G%d-%d", GeneXDim, GeneYDim];
    if(ClusterArrays) [basename appendFormat: @"_A%d-%d", ArrayXDim, ArrayYDim];

    filename = [basename stringByAppendingPathExtension: @"txt"];
    [[NSFileManager defaultManager] createFileAtPath: filename
                                            contents: nil
                                          attributes: nil];
    if (!OpenFile(&DataFile, filename, "wt")) {
        [statusbar setStringValue: @"Error: Unable to open the output file"];
        return;
    }

    if (ClusterGenes) {
        if((GeneIters<=0)||(GeneTau==0)||(GeneXDim==0)||(GeneYDim==0)) {
            CloseFile(DataFile);
            [statusbar setStringValue: @"Error starting SOM: Check options"];
            return;
	}
        filename = [basename stringByAppendingPathExtension: @"gnf"];
        [[NSFileManager defaultManager] createFileAtPath: filename
                                                contents: nil
                                              attributes: nil];
        if(!OpenFile(&GeneFile, filename, "wt")) {
            CloseFile(DataFile);
            [statusbar setStringValue: @"Error: Unable to open the output file"];
            return;
        }
    }

    if (ClusterArrays) {
        if((ArrayIters<=0)||(ArrayTau==0)||(ArrayXDim==0)||(ArrayYDim==0)) {
            [statusbar setStringValue: @"Error starting SOM: Check options"];
            CloseFile(DataFile);
            if(ClusterGenes) CloseFile(GeneFile);
            return;
        }
        filename = [basename stringByAppendingPathExtension: @"anf"];
        [[NSFileManager defaultManager] createFileAtPath: filename
                                                contents: nil
                                              attributes: nil];
        if (!OpenFile(&ArrayFile, filename, "wt")) {
            [statusbar setStringValue: @"Error: Unable to open the output file"];
            CloseFile(DataFile);
            if(ClusterGenes) CloseFile(GeneFile);
            return;
        }
    }

    int ok = PerformSOM(GeneFile.pointer, GeneXDim, GeneYDim,
                        GeneIters, GeneTau, GeneMetric,
                        ArrayFile.pointer, ArrayXDim, ArrayYDim,
                        ArrayIters, ArrayTau, ArrayMetric);
    if (ClusterGenes) CloseFile(GeneFile);
    if (ClusterArrays) CloseFile(ArrayFile);
    if (!ok) {
        NSRunCriticalAlertPanel(@"Insufficient memory",
                                @"Memory allocation problem",
                                @"OK",
                                nil,
                                nil);
        [statusbar setStringValue: @"Error performing SOM"];
        return;
    }

    ok = Save(DataFile.pointer, 0, 0);
    CloseFile(DataFile);
    if (!ok)
    {
        NSRunCriticalAlertPanel(@"Error saving file",
                                @"Insufficient memory",
                                @"OK",
                                nil,
                                nil);
        [statusbar setStringValue: @"Error saving to file"];
    }
    else
        [statusbar setStringValue: @"Done making SOM"];
}

- (IBAction)PCAExecute:(id)sender
{
    NSString* filename;
    FileHandle coordinatefile;
    FileHandle pcfile;
    const char* error;

    const BOOL DoGenePCA = [PCAGeneXB state];
    const BOOL DoArrayPCA = [PCAArrayXB state];

    if (!DoGenePCA && !DoArrayPCA) return; // Nothing to do

    const int rows = GetRows();
    const int columns = GetColumns();
    if (rows==0 || columns==0) {
        [statusbar setStringValue: @"No data available"];
        return;
    }

    NSString* base = [[FileMemo stringValue] stringByDeletingLastPathComponent];
    NSString* jobname = [base stringByAppendingPathComponent: [JobName stringValue]];

    if (DoGenePCA) {
        [statusbar setStringValue: @"Performing Principal Component Analysis"];
        [statusbar display];

        filename = [NSString stringWithString: jobname];
        filename = [filename stringByAppendingString: @"_pca_gene.coords"];
        filename = [filename stringByAppendingPathExtension: @"txt"];
        [[NSFileManager defaultManager] createFileAtPath: filename
                                                contents: nil
                                              attributes: nil];
        if (!OpenFile(&coordinatefile, filename, "wt")) {
            [statusbar setStringValue: @"Error: Unable to open the output file"];
            return;
        }

        filename = [NSString stringWithString: jobname];
        filename = [filename stringByAppendingString: @"_pca_gene.pc"];
        filename = [filename stringByAppendingPathExtension: @"txt"];
        [[NSFileManager defaultManager] createFileAtPath: filename
                                                contents: nil
                                              attributes: nil];
        if (!OpenFile(&pcfile, filename, "wt")) {
            CloseFile(coordinatefile);
            [statusbar setStringValue: @"Error: Unable to open the output file"];
            return;
        }
        error = PerformGenePCA(coordinatefile.pointer, pcfile.pointer);
        CloseFile(coordinatefile);
        CloseFile(pcfile);
        if (error) {
            [statusbar setStringValue: [NSString stringWithCString: error]];
            return;
        }
        [statusbar setStringValue: @"Finished Principal Component Analysis"];
    }
    if (DoArrayPCA) {
        [statusbar setStringValue: @"Performing Principal Component Analysis"];
        [statusbar display];

        filename = [NSString stringWithString: jobname];
        filename = [filename stringByAppendingString: @"_pca_array.coords"];
        filename = [filename stringByAppendingPathExtension: @"txt"];
        [[NSFileManager defaultManager] createFileAtPath: filename
                                                contents: nil
                                              attributes: nil];
        if (!OpenFile(&coordinatefile, filename, "wt")) {
            [statusbar setStringValue: @"Error: Unable to open the output file"];
            return;
        }

        filename = [NSString stringWithString: jobname];
        filename = [filename stringByAppendingString: @"_pca_array.pc"];
        filename = [filename stringByAppendingPathExtension: @"txt"];
        [[NSFileManager defaultManager] createFileAtPath: filename
                                                contents: nil
                                              attributes: nil];
        if (!OpenFile(&pcfile, filename, "wt")) {
            CloseFile(coordinatefile);
            [statusbar setStringValue: @"Error: Unable to open the output file"];
            return;
        }
        error = PerformArrayPCA(coordinatefile.pointer, pcfile.pointer);
        CloseFile(coordinatefile);
        CloseFile(pcfile);
        if (error) {
            [statusbar setStringValue: [NSString stringWithCString: error]];
            return;
        }
        [statusbar setStringValue: @"Finished Principal Component Analysis"];
    }
}

- (IBAction)ShowFileFormatPanel:(id)sender
{
    if(FileFormatPanel==nil) {
        if(![NSBundle loadNibNamed: @"FileFormatPanel.nib" owner:self] ) {
            NSLog(@"Load of FileFormatPanel.nib failed");
            return;
        }
    }
    [FileFormatPanel makeKeyAndOrderFront: nil];
}

- (IBAction)ShowAboutPanel:(id)sender
{
    if(AboutPanel==nil) {
        if(![NSBundle loadNibNamed: @"AboutPanel.nib" owner:self] ) {
            NSLog(@"Load of AboutPanel.nib failed");
            return;
        }
    }
    [AboutPanel makeKeyAndOrderFront: nil];
}

@end

@implementation Controller(ApplicationNotifications)
- (void)applicationDidFinishLaunching:(NSNotification *) aNotification
{
    [AdjustMeanGenes setEnabled: false];
    [AdjustMedianGenes setEnabled: false];
    [AdjustMeanArrays setEnabled: false];
    [AdjustMedianArrays setEnabled: false];
    [HierarchicalGeneWeight retain];
    [HierarchicalGeneWeight removeFromSuperview];
    [HierarchicalArrayWeight retain];
    [HierarchicalArrayWeight removeFromSuperview];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    Free();
}
@end
