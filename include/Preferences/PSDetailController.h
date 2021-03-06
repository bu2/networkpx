/*
 *     Generated by class-dump 3.1.2.
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2007 by Steve Nygard.
 */

#import <Preferences/PSViewController.h>

@class PSEditingPane, UIKeyboard, UIView, PSSpecifier;

@interface PSDetailController : PSViewController
{
    UIView *_view;	// 12 = 0xc
    PSEditingPane *_pane;	// 16 = 0x10
    UIKeyboard *_keyboard;	// 20 = 0x14
    BOOL _keyboardVisible;	// 24 = 0x18
}

- (void)_updateNavBarButtons;	// IMP=0x3176d084
- (id)initForContentSize:(CGSize)fp8;	// IMP=0x3176d0bc
- (void)dealloc;	// IMP=0x3176d1d4
- (void)viewWillRedisplay;	// IMP=0x3176d258
- (void)_addKeyboardView;	// IMP=0x3176d2d8
@property(retain) PSEditingPane* pane;
- (void)setKeyboardVisible:(BOOL)fp8 animated:(BOOL)fp12;	// IMP=0x3176d574
- (BOOL)keyboardVisible;	// IMP=0x3176d890
- (void)viewWillBecomeVisible:(PSSpecifier*)spec;	// IMP=0x3176d8a4
- (NSString*)navigationTitle;	// IMP=0x3176da4c
- (void)saveChanges;	// IMP=0x3176da88
- (void)suspend;	// IMP=0x3176db34
- (void)doneButtonClicked:(id)fp8;	// IMP=0x3176db50
- (void)cancelButtonClicked:(id)fp8;	// IMP=0x3176db94
- (void)navigationBarButtonClicked:(int)fp8;	// IMP=0x3176dbc4
- (BOOL)popController;	// IMP=0x3176dbec
@property(readonly) UIView* view;

@end

