#ifndef __MYUI_H__
#define __MYUI_H__

#include "lvgl.h"



static const lv_font_t *font;

static lv_obj_t *keyboard;
static lv_obj_t *label_name;
static lv_obj_t *label_pass;

static void return_to_parent_cb(lv_event_t *e);
void my_gui();
static void name_event_cb(lv_event_t *e);
static void pass_event_cb(lv_event_t *e);
static void keyboard_event_cb(lv_event_t *e);
void password_enter_face();
void loop_check_uart_and_update();


void Asend_button_event_cb(lv_event_t *e);
void uart2_send(const char *msg);
void device_offline_cb(lv_timer_t *timer);


#endif
