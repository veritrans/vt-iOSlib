//
//  VTBaseTransactionMethod.h
//  VTiOSAPI
//
//  Created by Muhammad Anis on 12/10/14.
//  Copyright (c) 2014 Veritrans Indonesia. All rights reserved.
//

#ifndef VTiOSAPI_VTBaseTransactionMethod_h
#define VTiOSAPI_VTBaseTransactionMethod_h

#import <Foundation/Foundation.h>
#import "VTBaseTransaction.h"

@interface VTBaseTransactionMethod : NSObject

@property(nonatomic,retain) VTBaseTransaction* transaction;

-(void)preAuthorize;
-(void)capture;
-(void)charge;

@end


#endif
