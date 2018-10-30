#include"client.h"
#include<gtk/gtk.h>

pthread_t recv_pid,snd_pid;
pthread_mutex_t mutex;
pthread_cond_t cond;
gchar content[1024];

/*全局变量声明区-----------------------------------------------------------------*/
	GtkWidget *Send_textview/*定义发送文本区*/,
	*Rcv_textview/*定义接收文本区*/;
	GtkTextBuffer *Send_buffer/*定义发送文本缓冲区*/,
	*Rcv_buffer/*定义接收文本缓冲区*/;

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void Clear_Local_message()
{
	GtkTextIter start,end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(Send_buffer),&start,&end);/*获得缓冲区开始和结束位置的Iter*/
	gtk_text_buffer_delete(GTK_TEXT_BUFFER(Send_buffer),&start,&end);/*插入到缓冲区*/
}
/*----------------------------------------------------------------------------*/
void Put_Local_message(const gchar *text)
{
	GtkTextIter start,end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(Rcv_buffer),&start,&end);/*获得缓冲区开始和结束位置的Iter*/
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(Rcv_buffer),&end,text,strlen(text));/*插入文本到缓冲区*/
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(Rcv_buffer),&end,"\n",1);/*插入文本到缓冲区*/
}
/*----------------------------------------------------------------------------*/
void Show_Err(char *err)
{
	GtkTextIter start,end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(Rcv_buffer),&start,&end);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(Rcv_buffer),&end,err,strlen(err));
}
/*----------------------------------------------------------------------------*/
/*发送函数----------------------------------------------------------------------*/
void on_send(GtkButton *SaveButton, GtkWidget *Send_textview)/*保存按钮的回调函数，每当‘保存’按钮被按下的时候，都会触发这个函数*/
{
	//GtkTextBuffer *S_buffer/*定义发送文本区缓冲区*/,*R_buffer/*定义接收文本缓冲区*/;
	GtkTextIter start,end;/*定义迭代器起点终点*/
	gchar *S_text,*R_text;/*定义文字存储变量*/

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(Send_buffer),&start,&end);
	S_text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(Send_buffer),&start,&end,FALSE);

	R_text = S_text;/*把发送文本交换到接收文本*/

	if(strcmp(R_text,"")!=0)
	{
		memset(content,0x00,strlen(content));
		Clear_Local_message();
		Put_Local_message(R_text);  
		strcpy(content,R_text); 
		pthread_cond_signal(&cond);
	}
	else
	{
		Show_Err("消息不能为空...\n");
	}
	free(R_text);
}
/*----------------------------------------------------------------------------*/
/*关闭函数---------------------------------------------------------------------*/
void on_close(GtkButton *CloseButton,GtkWidget *Send_textview)
{
	gtk_main_quit();
}
/*----------------------------------------------------------------------------*/
//读数据线程函数
void * pth_recv(void *arg)
{
	char buf[1024];
	int ser_pid = *((int *)arg);

	//一旦读取到数据，就会将数据显示到聊天界面上方的文本输入框
	while(1)
	{
		memset(buf,0x00,strlen(buf));
		
		if(read(ser_pid,buf,1024) > 0)
		{
			Put_Local_message((gchar *)buf); 
		}
	}

	return;
}
//写数据线程函数
void * pth_snd(void *arg)
{
	char buf[1024];
	int ser_pid = *((int *)arg);
	//将用户在聊天界面下方输入框内输入的字符发送给其他客户端
	//如果用户输入end,则结束程序
	while(1)
	{
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&cond,&mutex);
		strcpy(buf,(char *)content);
		if((strncmp(buf,"end",3) == 0))
		{
			gtk_main_quit();
			pthread_cancel(recv_pid);
			break;
		}
		write(ser_pid,buf,strlen(buf));
		pthread_mutex_unlock(&mutex);
	}

	return;
}


/*主函数-----------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
/*主函数变量声明区*/
	GtkWidget *window/*定义主窗口*/,
	*Send_scrolled_win/*定义发送滚动窗口*/,
	*Rcv_scrolled_win/*定义接收滚动窗口*/;
	GtkWidget *vbox/*定义垂直盒子*/;
	GtkWidget *Button_Box/*定义按钮盒*/,
	*SaveButton/*定义保存按钮*/,
	*CloseButton/*定义关闭按钮*/;
	GtkWidget *hseparator/*定义水平分割线*/;
	//*panel_v/*定义垂直分割面板*/,
	//*panel_h/*定义水平分割面板*/;

	gtk_init(&argc,&argv);/*GTK初始化*/
/*函数实现区*/

	/*------------------------------绘制主窗口----------------------------*/
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);/*生成主窗口*/
	g_signal_connect(G_OBJECT(window),"delete_event",G_CALLBACK(gtk_main_quit),NULL);/*连接信号，关闭窗口*/
	gtk_window_set_title(GTK_WINDOW(window),"Chat Box");/*设置主窗口标题*/
	gtk_container_set_border_width(GTK_CONTAINER(window),10);/*设置主窗口边框*/
	gtk_widget_set_size_request(window,500,500);/*设置主窗口初始化大小*/
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);/*设置主窗口初始位置*/
	/*------------------------------设置Send_text view-------------------------*/
	Send_textview = gtk_text_view_new();/*生成text view*/
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(Send_textview),GTK_WRAP_WORD);/*处理多行显示的模式*/
	gtk_text_view_set_justification(GTK_TEXT_VIEW(Send_textview),GTK_JUSTIFY_LEFT);/*控制文字显示方向的,对齐方式*/
	gtk_text_view_set_editable(GTK_TEXT_VIEW(Send_textview),TRUE);/*允许text view内容修改*/
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(Send_textview),TRUE);/*设置光标可见*/
	gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(Send_textview),5);/*设置上行距*/
	gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(Send_textview),5);/*设置下行距*/
	gtk_text_view_set_pixels_inside_wrap(GTK_TEXT_VIEW(Send_textview),5);/*设置词距*/
	gtk_text_view_set_left_margin(GTK_TEXT_VIEW(Send_textview),10);/*设置左边距*/
	gtk_text_view_set_right_margin(GTK_TEXT_VIEW(Send_textview),10);/*设置右边距*/
	Send_buffer =  gtk_text_view_get_buffer(GTK_TEXT_VIEW(Send_textview));/*返回text view被显示的buffer*/
	/*------------------------------设置Rcv_text view-------------------------*/
	Rcv_textview = gtk_text_view_new();/*生成text view*/
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(Rcv_textview),GTK_WRAP_WORD);/*处理多行显示的模式*/
	gtk_text_view_set_justification(GTK_TEXT_VIEW(Rcv_textview),GTK_JUSTIFY_LEFT);/*控制文字显示方向的,对齐方式*/
	gtk_text_view_set_editable(GTK_TEXT_VIEW(Rcv_textview),TRUE);/*允许text view内容修改*/
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(Rcv_textview),TRUE);/*设置光标可见*/
	gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(Rcv_textview),5);/*设置上行距*/
	gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(Rcv_textview),5);/*设置下行距*/
	gtk_text_view_set_pixels_inside_wrap(GTK_TEXT_VIEW(Rcv_textview),5);/*设置词距*/
	gtk_text_view_set_left_margin(GTK_TEXT_VIEW(Rcv_textview),10);/*设置左边距*/
	gtk_text_view_set_right_margin(GTK_TEXT_VIEW(Rcv_textview),10);/*设置右边距*/
	gtk_text_view_set_editable(GTK_TEXT_VIEW(Rcv_textview),FALSE);/*设置接收文字区不可被编辑*/
	Rcv_buffer =  gtk_text_view_get_buffer(GTK_TEXT_VIEW(Rcv_textview));/*返回text view被显示的buffer*/    
	/*------------------------------设置发送窗口滚动条-------------------------------*/
	Send_scrolled_win = gtk_scrolled_window_new(NULL,NULL);/*生成滚动条的窗口*/
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(Send_scrolled_win),Send_textview);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(Send_scrolled_win),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);/*滚动条属性*/
	/*------------------------------设置接收窗口滚动条-------------------------------*/
	Rcv_scrolled_win = gtk_scrolled_window_new(NULL,NULL);/*生成滚动条的窗口*/
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(Rcv_scrolled_win),Rcv_textview);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(Rcv_scrolled_win),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);/*滚动条属性*/
	/*------------------------------设置垂直盒子------------------------------*/
	vbox = gtk_vbox_new(FALSE,0);/*生成一个垂直排布的盒子*/
	/*------------------------------设置发送按钮------------------------------*/
	SaveButton = gtk_button_new_with_label("发送");/*生成发送按钮*/
	g_signal_connect(G_OBJECT(SaveButton),"clicked",G_CALLBACK(on_send),(gpointer)Send_textview);/*给按钮加上回调函数*/
	/*------------------------------设置关闭按钮------------------------------*/
	CloseButton = gtk_button_new_with_label("关闭");/*生成关闭按钮*/
	g_signal_connect(G_OBJECT(CloseButton),"clicked",G_CALLBACK(on_close),(gpointer)Send_textview);
	/*------------------------------设置按钮盒子------------------------------*/     
	Button_Box = gtk_hbutton_box_new();/*生成按钮盒*/
	gtk_box_set_spacing(GTK_BOX(Button_Box),1);/*按钮之间的间隔*/
	gtk_button_box_set_layout(GTK_BUTTON_BOX(Button_Box),GTK_BUTTONBOX_END);/*按钮盒内部布局，风格是尾对齐*/
	gtk_container_set_border_width(GTK_CONTAINER(Button_Box),5);/*边框宽*/
	/*------------------------------设置分割线--------------------------------*/
	hseparator = gtk_hseparator_new();
	/*------------------------------包装所有容器------------------------------*/
	gtk_container_add(GTK_CONTAINER(vbox),Rcv_scrolled_win);/*包装滚动条窗口到主窗口*/
	gtk_container_add(GTK_CONTAINER(vbox),hseparator);/*加入一条分割线*/
	gtk_container_add(GTK_CONTAINER(vbox),Send_scrolled_win);/*包装滚动条窗口到主窗口*/    
	gtk_container_add(GTK_CONTAINER(vbox),Button_Box);/*把按钮盒包装到vbox中*/
	gtk_box_pack_start(GTK_BOX(Button_Box),CloseButton,TRUE,TRUE,5);/*把关闭按钮包装到按钮盒里面去*/
	gtk_box_pack_start(GTK_BOX(Button_Box),SaveButton,TRUE,TRUE,5);/*把发送按钮包装到按钮盒里面去*/
	gtk_container_add(GTK_CONTAINER(window),vbox);/*将盒子封装到主窗口中去*/    
	/*------------------------------显示所有东西------------------------------*/
	gtk_widget_show_all(window);/*显示所有东西*/
	
	int cli_sid;
	char * ip = "127.0.0.1";
	unsigned short port = 5001;
	
	//连接服务器
	cli_sid = socket_connect(ip, port);

	//创建读数据线程
	if(pthread_create(&recv_pid,NULL,pth_recv,(void *)&cli_sid) != 0) 
		perror("pthread_create recv_pid error.");
	//创建写数据线程
	if(pthread_create(&snd_pid,NULL,pth_snd,(void *)&cli_sid) != 0)
		perror("pthread_create snd_pid error.");
	
	gtk_main();/*主循环*/
	
	//等待写数据线程结束
	if(pthread_join(snd_pid,NULL) != 0)
		perror("pthread_join snd_pid error.");
	//等待读数据线程结束
	if(pthread_join(recv_pid,NULL) != 0)
		perror("pthread_join recv_pid error.");
	
	//关闭连接上服务器的socket文件描述符
	socket_close(cli_sid);

	return 0;/*退出程序*/
    }
/*----------------------------------------------------------------------------*/
