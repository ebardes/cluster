/* Controller */

#import <Cocoa/Cocoa.h>

@interface Controller : NSObject
{
    IBOutlet id statusbar;
    IBOutlet id JobName;
    IBOutlet id FileMemo;
    IBOutlet id Rows;
    IBOutlet id Columns;
    IBOutlet id filterresult;
    IBOutlet id filteraccept;
    IBOutlet id FilterStdXB;
    IBOutlet id FilterPercentXB;
    IBOutlet id FilterObservationXB;
    IBOutlet id FilterMaxMinXB;
    IBOutlet id FilterStd;
    IBOutlet id FilterPercent;
    IBOutlet id FilterNumber;
    IBOutlet id FilterObservationValue;
    IBOutlet id FilterMaxMin;
    IBOutlet id AdjustLogXB;
    IBOutlet id AdjustNormalizeGenes;
    IBOutlet id AdjustNormalizeArrays;
    IBOutlet id AdjustCenterGenesXB;
    IBOutlet id AdjustMeanGenes;
    IBOutlet id AdjustMedianGenes;
    IBOutlet id AdjustCenterArraysXB;
    IBOutlet id AdjustMeanArrays;
    IBOutlet id AdjustMedianArrays;
    IBOutlet id HierarchicalGenes;
    IBOutlet id HierarchicalArrays;
    IBOutlet id HierarchicalGeneXB;
    IBOutlet id HierarchicalArrayXB;
    IBOutlet id HierarchicalGeneMetric;
    IBOutlet id HierarchicalArrayMetric;
    IBOutlet id HierarchicalGeneCutoff;
    IBOutlet id HierarchicalArrayCutoff;
    IBOutlet id HierarchicalGeneExp;
    IBOutlet id HierarchicalArrayExp;
    IBOutlet id HierarchicalGeneWeight;
    IBOutlet id HierarchicalArrayWeight;
    IBOutlet id HierarchicalGeneWeightXB;
    IBOutlet id HierarchicalArrayWeightXB;
    IBOutlet id KMeansGeneXB;
    IBOutlet id KMeansGeneK;
    IBOutlet id KMeansGeneMean;
    IBOutlet id KMeansGeneMetric;
    IBOutlet id KMeansGeneRuns;
    IBOutlet id KMeansArrayXB;
    IBOutlet id KMeansArrayK;
    IBOutlet id KMeansArrayMean;
    IBOutlet id KMeansArrayMetric;
    IBOutlet id KMeansArrayRuns;
    IBOutlet id SOMGeneXB;
    IBOutlet id SOMGeneIters;
    IBOutlet id SOMGeneTau;
    IBOutlet id SOMGeneMetric;
    IBOutlet id SOMGeneXDim;
    IBOutlet id SOMGeneYDim;
    IBOutlet id SOMArrayXB;
    IBOutlet id SOMArrayIters;
    IBOutlet id SOMArrayTau;
    IBOutlet id SOMArrayMetric;
    IBOutlet id SOMArrayXDim;
    IBOutlet id SOMArrayYDim;
	IBOutlet id PCAGeneXB;
    IBOutlet id PCAArrayXB;
    IBOutlet id FileFormatPanel;
    IBOutlet id AboutPanel;
    NSString* directory;
    int* use;
    int useRows;
}
- (void)UpdateInfo;
- (void)FilterReset;
- (char)GetMetric:(NSComboBox*)metricbox;
- (void)HierarchicalExecute:(NSString*)method;
- (IBAction)FileOpen:(id)sender;
- (IBAction)FileSave:(id)sender;
- (IBAction)ShowHelpManual:(id)sender;
- (IBAction)ShowHelpDownload:(id)sender;
- (IBAction)ShowFileFormatPanel:(id)sender;
- (IBAction)ShowAboutPanel:(id)sender;
- (IBAction)FilterApply:(id)sender;
- (IBAction)FilterAccept:(id)sender;
- (IBAction)AdjustApply:(id)sender;
- (IBAction)AdjustCenterGenesXBChanged:(id)sender;
- (IBAction)AdjustCenterArraysXBChanged:(id)sender;
- (IBAction)HierarchicalGeneWeightXBChanged:(id)sender;
- (IBAction)HierarchicalArrayWeightXBChanged:(id)sender;
- (IBAction)HierarchicalCentroid:(id)sender;
- (IBAction)HierarchicalSingle:(id)sender;
- (IBAction)HierarchicalComplete:(id)sender;
- (IBAction)HierarchicalAverage:(id)sender;
- (IBAction)KMeansExecute:(id)sender;
- (IBAction)KMeansExecute:(id)sender;
- (IBAction)SOMExecute:(id)sender;
- (IBAction)PCAExecute:(id)sender;
- (void)InitDir;
- (void)SaveDir:(NSString*)filename;
@end

@interface Controller(NSApplicationNotifications)
-(void)applicationDidFinishLaunching:(NSNotification*)notification;
@end
