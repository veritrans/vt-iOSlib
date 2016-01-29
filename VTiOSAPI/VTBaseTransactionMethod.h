//
//  VTBaseTransactionMethod.h
//  VTiOSAPI
//
//  Created by Muhammad Anis on 12/10/14.
//  Copyright (c) 2014 Veritrans Indonesia. All rights reserved.
//

#ifndef VTiOSAPI_VTBaseTransactionMethod_h
#define VTiOSAPI_VTBaseTransactionMethod_h

#define UnsupportedMethodException @"UnsupportedMethodException"

#import <Foundation/Foundation.h>
#import "VTBaseTransaction.h"

@interface VTBaseTransactionMethod : NSObject

@property(nonatomic,retain) VTBaseTransaction* transaction;

-(void)preAuthorize;
-(void)capture;
-(void)charge;
-(void)getToken:(void(^)(NSData* token, NSException* exception)) completionHandler;
-(void)registerCard:(void(^)(NSData* savedToken, NSException* exception)) completionHandler;
@end


#endif
