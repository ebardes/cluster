//
//  main.m
//  Cluster 3.0 for Mac OS X
//
//  Created by mdehoon on 17 October 2002.
//  Copyright (c) 2002, Michiel de Hoon. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "command.h"

int main(int argc, const char *argv[])
{
    int result;
    if ( (argc >= 2 && strncmp (argv[1], "-psn", 4) == 0 ) || (argc <= 1) )
        /* Double-clicking adds an additional argv, which starts with -psn */
        result = NSApplicationMain(argc, argv);
    else
        /* Run Cluster 3.0 as a command-line program */
        result = commandmain(argc, argv);
    return result;
}
