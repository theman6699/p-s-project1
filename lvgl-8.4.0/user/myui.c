#include "myui.h"
#include "lvgl.h"
#include "Font30.h"
#include "main.h"
#include "stdio.h"
#include <string.h>

//#define UART_RX_BUFFER_SIZE 512

//volatile uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];
//volatile uint16_t uart_rx_head = 0;
//volatile uint16_t uart_rx_tail = 0;

//static UART_HandleTypeDef *huart_global = NULL;
//static uint8_t rx_byte;


extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart2;
extern uint8_t rx_byte;
extern char rx_buffer[256];
extern uint16_t rx_index;
extern volatile uint8_t rx_ready;

int pass = 0;

static const lv_font_t *font;

static lv_obj_t *keyboard;
static lv_obj_t *label_name;
static lv_obj_t *label_pass;
static lv_obj_t *label_test;
static lv_obj_t *uart_textarea;
static lv_obj_t *send_textarea;
static lv_obj_t *Bdevice_status_label;

static lv_obj_t *Adevice_status_label;
static lv_timer_t *Adevice_watchdog_timer = NULL;
static lv_timer_t *Bdevice_watchdog_timer = NULL;
static lv_obj_t *Asendbtn;
static lv_obj_t *Bsendbtn;


lv_obj_t *chart;

lv_chart_series_t *ser;
lv_chart_series_t *ser1;


static lv_timer_t *btn1_timer = NULL;
static lv_timer_t *btn2_timer = NULL;


void main_control_face();

static lv_obj_t *parent = NULL;
static lv_obj_t *new_scr = NULL;


void simulate_btn1_cb(lv_timer_t *t)
{
    lv_event_send(Asendbtn, LV_EVENT_CLICKED, NULL);
}

void simulate_btn2_cb(lv_timer_t *t)
{
    lv_event_send(Bsendbtn, LV_EVENT_CLICKED, NULL);
}

//void clear_rx_buffer(void)
//{
//    memset(rx_buffer, 0, sizeof(rx_buffer));
//}

void sim900a_send_sms(void)
{
    const char *cmd1 = "AT+CMGF=1\r\n";  // 设置短信为文本模式
    const char *cmd2 = "AT+CMGS=\"15723044030\"\r\n"; // 设置收信号码
    const char *msg  = "DEVICEAFIRE";    // 短信正文
    const char ctrl_z = 0x1A;            // 发送结束符 Ctrl+Z

    HAL_UART_Transmit(&huart3, (uint8_t *)cmd1, strlen(cmd1), 1000);
    HAL_Delay(500);  // 等待模块响应

    HAL_UART_Transmit(&huart3, (uint8_t *)cmd2, strlen(cmd2), 1000);
    HAL_Delay(500);  // 等待模块准备

    HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), 1000);
    HAL_UART_Transmit(&huart3, (uint8_t *)&ctrl_z, 1, 1000);  // 发 Ctrl+Z 结束
}

void btn2_timer_start_cb(lv_timer_t *t)
{
    btn2_timer = lv_timer_create(simulate_btn2_cb, 3000, NULL);
    lv_timer_del(t);
}

void switch_event_cb(lv_event_t *e)
{
    lv_obj_t *sw = lv_event_get_target(e);

    if (lv_obj_has_state(sw, LV_STATE_CHECKED))
    {
        // 开启按钮1定时器
        btn1_timer = lv_timer_create(simulate_btn1_cb, 3000, NULL);

        // 延迟1.5秒后启动按钮2的周期定时器
        lv_timer_t *delay_timer = lv_timer_create(btn2_timer_start_cb, 1500, NULL);
    }
    else
    {
        if (btn1_timer) {
            lv_timer_del(btn1_timer);
            btn1_timer = NULL;
        }

        if (btn2_timer) {
            lv_timer_del(btn2_timer);
            btn2_timer = NULL;
        }
    }
}



void chart_update_ser(int new_value)
{
    lv_chart_set_next_value(chart, ser, new_value);
}

void chart_update_ser1(int new_value)
{
    lv_chart_set_next_value(chart, ser1, new_value);
}



void Adevice_offline_cb(lv_timer_t *timer)
{
    lv_label_set_text(Adevice_status_label, "设备A已离线");
}

void Bdevice_offline_cb(lv_timer_t *timer)
{
    lv_label_set_text(Bdevice_status_label, "设备B已离线");
}


void loop_check_uart_and_update()
{
if (rx_ready)
{
    rx_ready = 0;

    if (rx_buffer[0] == '@')
    {
        const char *payload = rx_buffer + 1; // 跳过 @

        // 显示到 TextArea
        char line[256];
        snprintf(line, sizeof(line), "rc: %s\n", payload);
        lv_textarea_add_text(uart_textarea, line);

int a_val1,a_val2,b_val1,b_val2;
			
			
if (sscanf(payload, "DEVICE_A_RECEIVED:%d,%d", &a_val1, &a_val2) == 2) {
    lv_label_set_text(Adevice_status_label, "设备A已连接");

    if (Adevice_watchdog_timer)
        lv_timer_del(Adevice_watchdog_timer);
    Adevice_watchdog_timer = lv_timer_create(Adevice_offline_cb, 3000, NULL);
		
		
		if(a_val2>90 || a_val1 > 70)
		{
			HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
			sim900a_send_sms();
			
		}
		else
		{
			HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
		}
		
		
		
		chart_update_ser(a_val1);
		chart_update_ser1(a_val2);
    // 打印数值用于调试
//    char dbg[64];
//    snprintf(dbg, sizeof(dbg), "A数据: %d, %d\n", a_val1, a_val2);
//    lv_textarea_add_text(uart_textarea, dbg);
}





        // ---------- 设备B ----------
        if (sscanf(payload, "DEVICE_B_RECEIVED:%d,%d", &b_val1, &b_val2) == 2)
        {
            lv_label_set_text(Bdevice_status_label, "设备B已连接");

            if (Bdevice_watchdog_timer)
                lv_timer_del(Bdevice_watchdog_timer);

            Bdevice_watchdog_timer = lv_timer_create(Bdevice_offline_cb, 3000, NULL);

            // 提取两个整数
//            int value1 = 0, value2 = 0;
//            sscanf(payload + 19, "%d,%d", &value1, &value2); // 跳过前缀
								if(b_val2>90 || b_val1 > 70)
								{
								HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
								}
								else
								{
								HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
								}
            return;
        }

        // TODO: 可添加更多格式的处理
    }
}

}









static void name_event_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *target = lv_event_get_target(e);
if(code == LV_EVENT_FOCUSED)
{
	lv_keyboard_set_textarea(keyboard,target);
}	
else if(code == LV_EVENT_VALUE_CHANGED)
{
	const char *txt = lv_textarea_get_text(target);
	
	if(strcmp(txt,"admin") == 0)
	{
		lv_label_set_text(label_name, LV_SYMBOL_OK);
	}
	else{
		lv_label_set_text(label_name, "");
	}
	
}

}

static void pass_event_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *target = lv_event_get_target(e);
if(code == LV_EVENT_FOCUSED)
{
	lv_keyboard_set_textarea(keyboard,target);
}	
else if(code == LV_EVENT_VALUE_CHANGED)
{
	const char *txt = lv_textarea_get_text(target);
	
	if(strcmp(txt,"123456") == 0)
	{
		lv_label_set_text(label_pass, LV_SYMBOL_OK);
	}
	else{
		lv_label_set_text(label_pass, "");
	}
	
}

}

static void keyboard_event_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *target = lv_event_get_target(e);
if(code == LV_EVENT_VALUE_CHANGED)
{
	uint16_t id = lv_btnmatrix_get_selected_btn(target);
	const char *txt = lv_btnmatrix_get_btn_text(target,id);
	
	if(strcmp(txt, LV_SYMBOL_KEYBOARD) == 0)
	{
		if(lv_keyboard_get_mode(target) == LV_KEYBOARD_MODE_NUMBER)
		{
			lv_keyboard_set_mode(target,LV_KEYBOARD_MODE_TEXT_LOWER);
		}
		else{
			lv_keyboard_set_mode(target,LV_KEYBOARD_MODE_NUMBER);
		}
		
	}
	else if(strcmp(txt, LV_SYMBOL_OK) == 0)
	{
		const char* nametxt = lv_label_get_text(label_name);
		const char* passtxt = lv_label_get_text(label_pass);
		
		if((strcmp(nametxt, LV_SYMBOL_OK) == 0)&&(strcmp(passtxt, LV_SYMBOL_OK) == 0))
		{
			main_control_face();
		}
		
		
	}
}
}




static void return_to_parent_cb(lv_event_t *e) {
    if (parent != NULL) {
        lv_scr_load_anim(parent, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false);
    }
}

void uart2_send(const char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 100);
}



void Asend_button_event_cb(lv_event_t *e)
{
    char msg[64] = "";
		snprintf(msg,sizeof(msg),"@%s\n","DEVICE_A_REC?");
    uart2_send(msg);
	
		char line[128];
		snprintf(line, sizeof(line), "tx: %s", msg);
		lv_textarea_add_text(send_textarea, line);
		
		
		
		
}

void Bsend_button_event_cb(lv_event_t *e)
{
    char msg[64] = "";
		snprintf(msg,sizeof(msg),"@%s\n","DEVICE_B_REC?");
    uart2_send(msg);
	
		char line[128];
		snprintf(line, sizeof(line), "tx: %s", msg);
		lv_textarea_add_text(send_textarea, line);
		
		
		
		
}


void timer_cb(lv_timer_t * timer)
{
    static int val = 0;
    val = (val + 1) % 100; // 模拟数据
    chart_update_ser(val);
}

void start_chart_timer(void)
{
    lv_timer_create(timer_cb, 100, NULL); // 每100ms调用一次
}




void main_control_face()
{
	new_scr = lv_obj_create(NULL); // NULL 代表根对象
	lv_obj_set_style_bg_color(new_scr, lv_color_hex(0xCC9900), LV_PART_MAIN);

	//lv_scr_load(new_scr); // 加载新界面
	lv_scr_load_anim(new_scr,LV_SCR_LOAD_ANIM_MOVE_LEFT,500,10,false);
// 添加控件
	lv_obj_t *logout_btn = lv_btn_create(new_scr);
	lv_obj_align(logout_btn, LV_ALIGN_TOP_RIGHT, 0, 20);  // 左上角对齐，偏移量为 (0, 0)
	
	lv_obj_t *logout_label = lv_label_create(logout_btn);
	lv_obj_set_style_text_font(logout_label, &Font30, LV_STATE_DEFAULT);
	lv_label_set_text(logout_label,"登出");
	lv_obj_add_event_cb(logout_btn,return_to_parent_cb,LV_EVENT_CLICKED,NULL);
	
	lv_obj_t *welcome_title_label = lv_label_create(new_scr);
	lv_obj_set_style_text_font(welcome_title_label, &Font30, LV_STATE_DEFAULT);
  lv_label_set_text(welcome_title_label, "你好，用户admin");
  lv_obj_align(welcome_title_label, LV_ALIGN_TOP_LEFT, 320, 10); 
	
	//create_send_button();
	lv_obj_t *send_title_label = lv_label_create(new_scr);
	lv_obj_set_style_text_font(send_title_label, &Font30, LV_STATE_DEFAULT);
  lv_label_set_text(send_title_label, "数据发送区");
  lv_obj_align(send_title_label, LV_ALIGN_TOP_LEFT, 10, 230); 
	
	send_textarea = lv_textarea_create(new_scr);
  lv_obj_set_size(send_textarea, 300, 200);
  lv_obj_align(send_textarea, LV_ALIGN_BOTTOM_LEFT, 10, -10);
  //lv_textarea_set_text(send_textarea, "");
  //lv_textarea_set_cursor_hidden(send_textarea, true);
  lv_obj_clear_flag(send_textarea, LV_OBJ_FLAG_CLICKABLE);
	
	
	
	
	Asendbtn = lv_btn_create(new_scr);
  lv_obj_align(Asendbtn, LV_ALIGN_TOP_LEFT, 320, 50);  // you下角
  lv_obj_t *Alabel = lv_label_create(Asendbtn);
	lv_obj_set_style_text_font(Alabel, &Font30, LV_STATE_DEFAULT);
  lv_label_set_text(Alabel, "请求设备A回应");
  lv_obj_add_event_cb(Asendbtn, Asend_button_event_cb, LV_EVENT_CLICKED, NULL);
	
	Bsendbtn = lv_btn_create(new_scr);
  lv_obj_align(Bsendbtn, LV_ALIGN_TOP_LEFT, 320, 180);  // you下角
  lv_obj_t *Blabel = lv_label_create(Bsendbtn);
	lv_obj_set_style_text_font(Blabel, &Font30, LV_STATE_DEFAULT);
  lv_label_set_text(Blabel, "请求设备B回应");
  lv_obj_add_event_cb(Bsendbtn, Bsend_button_event_cb, LV_EVENT_CLICKED, NULL);
	
	
	
	
	
	
// label_test = lv_label_create(new_scr);
// lv_label_set_text(label_test, "Waiting...");
// lv_obj_align(label_test, LV_ALIGN_TOP_LEFT, 10, 50);  // 在屏幕上方偏下显示

		lv_obj_t *receive_title_label = lv_label_create(new_scr);
		lv_obj_set_style_text_font(receive_title_label, &Font30, LV_STATE_DEFAULT);
    lv_label_set_text(receive_title_label, "数据接收区");
    lv_obj_align(receive_title_label, LV_ALIGN_TOP_LEFT, 10, 0); 
		
		uart_textarea = lv_textarea_create(new_scr);
    lv_obj_set_size(uart_textarea, 300, 200);
    lv_obj_align(uart_textarea, LV_ALIGN_TOP_LEFT, 10,30);
    lv_textarea_set_max_length(uart_textarea, 1024); // 限制总长度
    lv_textarea_set_text(uart_textarea, "");         // 初始为空
    //lv_textarea_set_cursor_hidden(uart_textarea, true); // 隐藏光标
    lv_obj_clear_flag(uart_textarea, LV_OBJ_FLAG_CLICKABLE); // 不允许输入
		
		
		Adevice_status_label = lv_label_create(new_scr);
		lv_obj_set_style_text_font(Adevice_status_label, &Font30, LV_STATE_DEFAULT);
		lv_label_set_text(Adevice_status_label, "设备A状态未知");
		lv_obj_align(Adevice_status_label, LV_ALIGN_TOP_LEFT, 320, 130);  // 居中
		
		Bdevice_status_label = lv_label_create(new_scr);
		lv_obj_set_style_text_font(Bdevice_status_label, &Font30, LV_STATE_DEFAULT);
		lv_label_set_text(Bdevice_status_label, "设备B状态未知");
		lv_obj_align(Bdevice_status_label, LV_ALIGN_TOP_LEFT, 320, 250); 
		
		
		
//		device_status_label = lv_label_create(new_scr);
//		lv_label_set_text(device_status_label, "");  // 初始为空
//		lv_obj_align(device_status_label, LV_ALIGN_BOTTOM_LEFT, 10, 80);  // 左下角上方一点

		
		lv_obj_t *circle_title_label = lv_label_create(new_scr);
		lv_obj_set_style_text_font(circle_title_label, &Font30, LV_STATE_DEFAULT);
		lv_label_set_text(circle_title_label, "循环询问是否在线");
		lv_obj_align(circle_title_label, LV_ALIGN_TOP_RIGHT, 0, 90); 
		
		
		
		lv_obj_t *sw = lv_switch_create(new_scr);
		lv_obj_align(sw, LV_ALIGN_TOP_RIGHT, -100, 120);
		lv_obj_set_size(sw, 100, 50);
		lv_obj_add_event_cb(sw, switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
		
		
		chart = lv_chart_create(new_scr);
    lv_obj_set_size(chart, 200, 150);
    lv_obj_align(chart, LV_ALIGN_BOTTOM_RIGHT, -210, -30);

    // 设置图表样式
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE); // 折线图
    lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT); // 移动数据
    lv_chart_set_point_count(chart, 50); // 每条曲线最多显示 50 个点
		lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
		lv_chart_set_point_count(chart, 50);
		lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);

		lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 10, 5, 6, 1, true, 40);
		lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 10, 5, 5, 1, true, 40);

//		lv_chart_set_axis_tick_texts(chart, LV_CHART_AXIS_PRIMARY_X,
//    "0\n10\n20\n30\n40\n50", LV_CHART_AXIS_DRAW_LAST_TICK);

//		lv_chart_set_axis_tick_texts(chart, LV_CHART_AXIS_PRIMARY_Y,
//    "0\n25\n50\n75\n100", LV_CHART_AXIS_DRAW_LAST_TICK);
    // 添加一条数据曲线
    ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
		ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
    // 初始化数据
    for (int i = 0; i < 50; i++) {
        lv_chart_set_next_value(chart, ser, 0);
				lv_chart_set_next_value(chart, ser1, 0);
    }
		
		//start_chart_timer();
}


void password_enter_face()
{
	//font = &LV_FONT_MONTSERRAT_22;
	
	parent = lv_scr_act();
	lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xCC9900), LV_PART_MAIN);

	
	lv_obj_t *title_label = lv_label_create(parent);
	lv_obj_set_style_text_font(title_label, &Font30, LV_STATE_DEFAULT);
  lv_label_set_text(title_label, "基于STM32H750XBH6的火灾报警系统");
  lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 60, 5); 
	
	lv_obj_t *sub_title_label = lv_label_create(parent);
	lv_obj_set_style_text_font(sub_title_label, &Font30, LV_STATE_DEFAULT);
  lv_label_set_text(sub_title_label, "登录");
  lv_obj_align(sub_title_label, LV_ALIGN_TOP_LEFT, 60, 70); 
	
	
	
	lv_obj_t *textarea_name = lv_textarea_create(parent); 
	lv_obj_set_width(textarea_name, 400);
	//lv_obj_set_style_text_font(textarea_name,LV_FONT_MONTSERRAT_22,LV_PART_MAIN);
	lv_obj_align(textarea_name,LV_ALIGN_CENTER,0,-160);
	
	lv_textarea_set_one_line(textarea_name,true);
	lv_textarea_set_max_length(textarea_name,6);
	lv_textarea_set_placeholder_text(textarea_name,"user name");
	lv_obj_add_event_cb(textarea_name,name_event_cb,LV_EVENT_ALL,NULL);
	
	
	label_name = lv_label_create(parent);
	lv_label_set_text(label_name,"");
	//lv_obj_set_style_text_font(label_name,LV_FONT_MONTSERRAT_22,LV_PART_MAIN);
	lv_obj_align_to(label_name,textarea_name,LV_ALIGN_OUT_RIGHT_MID,5,0);
	
	lv_obj_t *textarea_pass = lv_textarea_create(parent);
	lv_obj_set_width(textarea_pass, 400);
	//lv_obj_set_style_text_font(textarea_pass,LV_FONT_MONTSERRAT_22,LV_PART_MAIN);
	lv_obj_align_to(textarea_pass,textarea_name,LV_ALIGN_OUT_BOTTOM_MID,0,24);
	lv_textarea_set_one_line(textarea_pass,true);
	lv_textarea_set_password_mode(textarea_pass,true);
	lv_textarea_set_password_show_time(textarea_pass,1000);
	lv_textarea_set_max_length(textarea_pass,8);
	lv_textarea_set_placeholder_text(textarea_pass,"password");
	lv_obj_add_event_cb(textarea_pass,pass_event_cb,LV_EVENT_ALL,NULL);
	
	
	label_pass = lv_label_create(parent);
	lv_label_set_text(label_pass,"");
	//lv_obj_set_style_text_font(label_pass,LV_FONT_MONTSERRAT_22,LV_PART_MAIN);
	lv_obj_align_to(label_pass,textarea_pass,LV_ALIGN_OUT_RIGHT_MID,5,0);
	
	keyboard = lv_keyboard_create(parent);
	lv_obj_set_size(keyboard,800,240);
	lv_obj_add_event_cb(keyboard,keyboard_event_cb,LV_EVENT_VALUE_CHANGED,NULL);
	
	
	
}


void my_gui()
{
	password_enter_face();
	
}
