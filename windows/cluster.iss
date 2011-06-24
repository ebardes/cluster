; -- cluster.iss --
; The installer script for Cluster 3.0.
; Michiel de Hoon, 2002.06.15

[Setup]
AppName=Cluster
AppVerName=Cluster 3.0
AppCopyright=Copyright (C) 2004 Michiel de Hoon
OutputBaseFilename=clustersetup
DefaultDirName={pf}\Stanford University\
DefaultGroupName=Cluster
UninstallDisplayIcon=
AppPublisher=Michiel de Hoon, University of Tokyo
AppPublisherURL=http://bonsai.ims.u-tokyo.ac.jp/~mdehoon
AppVersion=3.0
OutputDir=.
; uncomment the following line if you want your installation to run on NT 3.51 too.
; MinVersion=4,3.51

[Files]
Source: "cluster.exe"; DestDir: "{app}\Cluster 3.0"; MinVersion: 0, 1
; Windows NT, 2000, XP
Source: "clust95.exe"; DestDir: "{app}\Cluster 3.0"; DestName: "cluster.exe"; MinVersion: 1, 0
; WIndows 95, 98, Me
Source: "cluster.com"; DestDir: "{app}\Cluster 3.0"; MinVersion: 0, 1
; Windows NT, 2000, XP
Source: "clust95.com"; DestDir: "{app}\Cluster 3.0"; DestName: "cluster.com"; MinVersion: 1, 0
; WIndows 95, 98, Me
Source: "cluster.dll"; DestDir: "{app}\Cluster 3.0"
Source: "../doc/cluster3.pdf"; DestDir: "{app}\Cluster 3.0\doc"
Source: "cluster.chm"; DestDir: "{app}\Cluster 3.0"

[Icons]
Name: "{group}\Cluster 3.0"; Filename: "{app}\Cluster 3.0\cluster.exe"

[Registry]
Root: HKCU; Subkey: "Software\Stanford\Cluster\Home"; Flags: createvalueifdoesntexist; ValueType: string; ValueName: ClusterDirectory; ValueData: {app}
Root: HKCU; Subkey: "Software\Stanford\Cluster\Directory"; Flags: createvalueifdoesntexist; ValueType: string; ValueName: LastOpenDirectory; ValueData: {app}
Root: HKCU; Subkey: "Software\Stanford\Cluster\WeightSettings"; Flags: createvalueifdoesntexist; ValueType: binary; ValueName: ArrayWeightCutoff; ValueData: "9A 99 99 99 99 B9 3F"
Root: HKCU; Subkey: "Software\Stanford\Cluster\WeightSettings"; Flags: createvalueifdoesntexist; ValueType: binary; ValueName: ArrayWeightExp; ValueData: "00 00 00 00 00 F0 3F"
Root: HKCU; Subkey: "Software\Stanford\Cluster\WeightSettings"; Flags: createvalueifdoesntexist; ValueType: binary; ValueName: GeneWeightCutoff; ValueData: "9A 99 99 99 99 B9 3F"
Root: HKCU; Subkey: "Software\Stanford\Cluster\WeightSettings"; Flags: createvalueifdoesntexist; ValueType: binary; ValueName: GeneWeightExp; ValueData: "00 00 00 00 00 F0 3F"

