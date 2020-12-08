/**    chat_client **/
 
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<sys/socket.h>
#include <sys/types.h>
#include<pthread.h>
#include<time.h>
#include <gtk/gtk.h>

#define BUF_SIZE 100
#define NORMAL_SIZE 20
 
void* send_msg(void* arg);
void* recv_msg(void* arg);
void error_handling(char* msg);
 
void menu();
void changeName();
void menuOptions(); 
static void delete(GtkWidget *widget, gpointer data);
static void gui(GtkWidget *widget, GList *initlist, gpointer data);
static int client(int domain, int type, int protocol, struct sockaddr *serv_addr, int addrlen);
static int server(int sock, int domain, int type, int protocol, struct sockaddr *serv_addr, int addrlen, int backlog, struct sockaddr_storage client_addr, socklen_t addr_size);
 
char name[NORMAL_SIZE]="[DEFALT]";     // name
char msg_form[NORMAL_SIZE];            // msg form
char serv_time[NORMAL_SIZE];        // server time
char msg[BUF_SIZE];                    // msg
char serv_port[NORMAL_SIZE];        // server port number
char clnt_ip[NORMAL_SIZE];            // client ip address
 
 
int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void* thread_return;
    
    if (argc!=4)
    {
        printf(" Usage : %s <ip> <port> <name>\n", argv[0]);
        exit(1);
    }
 
 	GtkWidget *initwindow, *initbox,
		*namebox, *namelabel, *nameentry,
		//*firstbox, *firstlabel, *firstentry,
		*radiobox, *clientradio, *serverradio,
		*ipbox, *iplabel, *ipentry,
		*portbox, *portlabel, *portentry,
		*buttonbox, *initbutton;
	GList *initlist, *radiolist;
	gboolean tru, lie;
	tru = TRUE;
	lie = FALSE;
	initlist=NULL; radiolist=NULL;
	
	gtk_init(&argc, &argv);

	initwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (initwindow), "UI Chat Initialization");
	g_signal_connect(initwindow, "destroy", G_CALLBACK (delete), NULL);
	initbox = gtk_vbox_new(lie, 0);
	gtk_container_add(GTK_CONTAINER (initwindow), initbox);

	namebox = gtk_hbox_new(tru, 2);
	namelabel = gtk_label_new("Your name: ");
	nameentry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX (namebox), namelabel, lie, tru, 0);
	gtk_box_pack_start(GTK_BOX (namebox), nameentry, tru, tru, 0);
 
 	namebox = gtk_hbox_new(tru, 2);
  namelabel = gtk_label_new("Your name: ");
  nameentry = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX (namebox), namelabel, lie, tru, 0);
  gtk_box_pack_start(GTK_BOX (namebox), nameentry, tru, tru, 0);
/*
  firstbox = gtk_hbox_new(tru, 2);
  firstlabel = gtk_label_new("First message: ");
  firstentry = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX (firstbox), firstlabel, lie, tru, 0);
  gtk_box_pack_start(GTK_BOX (firstbox), firstentry, tru, tru, 0);
*/
  radiobox = gtk_hbox_new(tru, 2);
  serverradio = gtk_radio_button_new_with_label(NULL, "Server");
  clientradio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(serverradio), "Client");
  gtk_box_pack_start(GTK_BOX (radiobox), clientradio, lie, tru, 0);
  gtk_box_pack_start(GTK_BOX (radiobox), serverradio, lie, tru, 0);

  ipbox = gtk_hbox_new(tru, 2);
  iplabel = gtk_label_new("IP Address to connect to: ");
  ipentry = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX (ipbox), iplabel, lie, tru, 0);
  gtk_box_pack_start(GTK_BOX (ipbox), ipentry, tru, tru, 0);

  portbox = gtk_hbox_new(tru, 2);
  portlabel = gtk_label_new("Port: ");
  portentry = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX (portbox), portlabel, lie, tru, 0);
  gtk_box_pack_start(GTK_BOX (portbox), portentry, tru, tru, 0);

  buttonbox = gtk_hbox_new(tru, 2);
  initbutton = gtk_button_new_with_label("CONNECT!");
  gtk_box_pack_start(GTK_BOX (buttonbox), initbutton, tru, tru, 0);

  gtk_box_pack_start(GTK_BOX (initbox), namebox, tru, tru, 0);
  //gtk_box_pack_start(GTK_BOX (initbox), firstbox, tru, tru, 0);
  gtk_box_pack_start(GTK_BOX (initbox), radiobox, tru, tru, 0);
  gtk_box_pack_start(GTK_BOX (initbox), ipbox, tru, tru, 0);
  gtk_box_pack_start(GTK_BOX (initbox), portbox, tru, tru, 0);
  gtk_box_pack_start(GTK_BOX (initbox), buttonbox, tru, tru, 0);

  initlist = g_list_append(initlist, initwindow);
  initlist = g_list_append(initlist, nameentry);
  //initlist = g_list_append(initlist, firstentry);
  initlist = g_list_append(initlist, portentry);
  initlist = g_list_append(initlist, clientradio);
  initlist = g_list_append(initlist, ipentry);
  g_signal_connect(initbutton, "clicked", G_CALLBACK (gui), initlist);

  gtk_widget_show_all(initwindow);
  gtk_main();
 	
 
    /** local time **/
    struct tm *t;
    time_t timer = time(NULL);
    t=localtime(&timer);
    sprintf(serv_time, "%d-%d-%d %d:%d", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour,
    t->tm_min);
 
    sprintf(name, "[%s]", argv[3]);
    sprintf(clnt_ip, "%s", argv[1]);
    sprintf(serv_port, "%s", argv[2]);
    sock=socket(PF_INET, SOCK_STREAM, 0);
 
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_addr.sin_port=htons(atoi(argv[2]));
 
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
        error_handling(" conncet() error");
 
    /** call menu **/
    menu();
 
    pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);
    close(sock);
    return 0;
}
 
void* send_msg(void* arg)
{
    int sock=*((int*)arg);
    char name_msg[NORMAL_SIZE+BUF_SIZE];
    char myInfo[BUF_SIZE];
    char* who = NULL;
    char temp[BUF_SIZE];
 
    /** send join messge **/
    printf(" >> join the chat !! \n");
    sprintf(myInfo, "%s's join. IP_%s\n",name , clnt_ip);
    write(sock, myInfo, strlen(myInfo));
 
    while(1)
    {
        fgets(msg, BUF_SIZE, stdin);
 
        // menu_mode command -> !menu
        if (!strcmp(msg, "!menu\n"))
        {
            menuOptions();
            continue;
        }
 
        else if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
        {
            close(sock);
            exit(0);
        }
 
        // send message
        sprintf(name_msg, "%s %s", name,msg);
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
}
 
void* recv_msg(void* arg)
{
    int sock=*((int*)arg);
    char name_msg[NORMAL_SIZE+BUF_SIZE];
    int str_len;
 
    while(1)
    {
        str_len=read(sock, name_msg, NORMAL_SIZE+BUF_SIZE-1);
        if (str_len==-1)
            return (void*)-1;
        name_msg[str_len]=0;
        fputs(name_msg, stdout);
    }
    return NULL;
}
 
 
void menuOptions() 
{
    int select;
    // print menu
    printf("\n\t**** menu mode ****\n");
    printf("\t1. change name\n");
    printf("\t2. clear/update\n\n");
    printf("\tthe other key is cancel");
    printf("\n\t*******************");
    printf("\n\t>> ");
    scanf("%d", &select);
    getchar();
    switch(select)
    {
        // change user name
        case 1:
        changeName();
        break;
 
        // console update(time, clear chatting log)
        case 2:
        menu();
        break;
 
        // menu error
        default:
        printf("\tcancel.");
        break;
    }
}
 
 
/** change user name **/
void changeName()
{
    char nameTemp[100];
    printf("\n\tInput new name -> ");
    scanf("%s", nameTemp);
    sprintf(name, "[%s]", nameTemp);
    printf("\n\tComplete.\n\n");
}
 
void menu()
{
    system("clear");
    printf(" **** moon/sum chatting client ****\n");
    printf(" server port : %s \n", serv_port);
    printf(" client IP   : %s \n", clnt_ip);
    printf(" chat name   : %s \n", name);
    printf(" server time : %s \n", serv_time);
    printf(" ************* menu ***************\n");
    printf(" if you want to select menu -> !menu\n");
    printf(" 1. change name\n");
    printf(" 2. clear/update\n");
    printf(" **********************************\n");
    printf(" Exit -> q & Q\n\n");
}    
 
void error_handling(char* msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

static void delete(GtkWidget *widget, gpointer data) {
  gtk_main_quit();
}

static void gui(GtkWidget *widget, GList *initlist, gpointer data) {
  GtkWidget *mainwindow, *mainbox, *mainscroll, *mainbutton, *mainhbox, *mainview, *mainmessage, *mainlabel;
  GtkTextBuffer *mainbuffer;
  GtkTextIter iter[2];
  GtkAdjustment *vertadjust;
  GIOChannel *clientio;
  GList *mainlist, *recvlist;
  gboolean tru, lie;
  pthread_t recvthread;
  int sendstat, backlog;
  gboolean isclient;
  char *buffer;
  gchar *text, *message;
  buffer = malloc(256 * sizeof(char));
  backlog=10;
  isclient=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_list_nth_data(initlist, 3)));
  mainlist=NULL; recvlist=NULL;
  tru = TRUE;
  lie = FALSE;
  const gchar *port, *ip;
  port = malloc(10 * sizeof(char));
  ip = malloc(20 * sizeof(char));
  sendstat = 0;
  port = gtk_entry_get_text(GTK_ENTRY(g_list_nth_data(initlist, 2)));
  if (isclient) 
    ip = gtk_entry_get_text(GTK_ENTRY(g_list_nth_data(initlist, 4)));

  /* Begin Berkeley Socket Stuff */

  struct sockaddr_storage client_addr;
  socklen_t addr_size;
  struct addrinfo hints, *res;
  int sock, client_sock, listencount;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if (isclient) {
    getaddrinfo( ip, port, &hints, &res);
  } else {
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, port, &hints, &res);
  }

  if (isclient) {
    client_sock = client(res->ai_family, res->ai_socktype, res->ai_protocol, res->ai_addr, res->ai_addrlen);
    listencount = 0;
  } else {
    client_sock = server(sock, res->ai_family, res->ai_socktype, res->ai_protocol, res->ai_addr, res->ai_addrlen, backlog, client_addr, addr_size);
    listencount = 1;
  }

  recvlist = g_list_append(recvlist, &client_sock);
  recvlist = g_list_append(recvlist, mainbuffer);
  pthread_create(&recvthread, NULL, recv_msg, recvlist);

  /* Back to your regularly scheduled GUI stuff */

  gtk_widget_hide(g_list_nth_data(initlist, 0));
  mainwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect(mainwindow, "destroy", G_CALLBACK (delete), NULL);
  gtk_window_set_default_size(GTK_WINDOW (mainwindow), 400, 400);
  gtk_window_set_title(GTK_WINDOW (mainwindow), "UI Chat");
  gtk_widget_show(mainwindow);

  mainbox = gtk_vbox_new(lie, 0);
  mainhbox = gtk_hbox_new(lie, 0);
  mainscroll = gtk_scrolled_window_new(NULL, NULL);
  mainbuffer = gtk_text_buffer_new(NULL);
  mainview = gtk_text_view_new_with_buffer(mainbuffer);
  mainlabel = gtk_label_new(gtk_entry_get_text(GTK_ENTRY(g_list_nth_data(initlist, 1))));
  mainmessage = gtk_entry_new();
  mainbutton = gtk_button_new_with_label("Send");
  vertadjust = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(mainscroll));

  gtk_container_add(GTK_CONTAINER (mainwindow), mainbox);
  gtk_box_pack_start(GTK_BOX (mainbox), mainscroll, tru, tru, 0);
  gtk_box_pack_start(GTK_BOX (mainbox), mainhbox, lie, lie, 0);
  gtk_box_pack_start(GTK_BOX (mainhbox), mainlabel, lie, lie, 0);
  gtk_box_pack_start(GTK_BOX (mainhbox), mainmessage, tru, tru, 0);
  gtk_box_pack_start(GTK_BOX (mainhbox), mainbutton, lie, lie, 0);
  
  gtk_text_view_set_editable(GTK_TEXT_VIEW (mainview), lie);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW (mainview), GTK_WRAP_WORD_CHAR);
  gtk_container_add(GTK_CONTAINER (mainscroll), mainview);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (mainscroll), 1, 1);
  //gtk_text_buffer_set_text(mainbuffer, first, -1);

  recv(client_sock, buffer, 256, 0);
  gtk_text_buffer_get_iter_at_offset(mainbuffer, &iter[0], 0);
  gtk_text_buffer_get_iter_at_offset(mainbuffer, &iter[1], -1);
  message = buffer;
  text = g_strconcat(gtk_text_buffer_get_text(mainbuffer, &iter[0], &iter[1], TRUE), message, NULL);
  gtk_text_buffer_set_text(mainbuffer, text, -1);

  mainlist = g_list_append(mainlist, mainview);
  mainlist = g_list_append(mainlist, mainlabel);
  mainlist = g_list_append(mainlist, mainmessage);
  mainlist = g_list_append(mainlist, vertadjust);
  mainlist = g_list_append(mainlist, &client_sock);
  g_signal_connect(mainbutton, "clicked", G_CALLBACK (send_msg), mainlist);

  gtk_widget_show_all(mainwindow);
}

static int client(int domain, int type, int protocol, struct sockaddr *serv_addr, int addrlen) {
  int returnvalue;
  returnvalue = socket(domain, type, protocol);
  connect(returnvalue, serv_addr, addrlen);
  return returnvalue;
}

static int server(int sock, int domain, int type, int protocol, struct sockaddr *serv_addr, int addrlen, int backlog, struct sockaddr_storage client_addr, socklen_t addr_size) {
  int returnvalue;
  sock = socket(domain, type, protocol);
  bind(sock, serv_addr, addrlen);
  listen(sock, backlog);
  addr_size = sizeof client_addr;
  returnvalue = accept(sock, (struct sockaddr *)&client_addr, &addr_size);
  return returnvalue;
}
