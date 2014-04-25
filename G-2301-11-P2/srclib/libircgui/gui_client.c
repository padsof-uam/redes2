/***************************************************************************************************
* Redes de Comunicaciones II                                                                       *
* 25/2/2014                                                                                        *
*                                                                                                  *
*    Interfaz para el cliente IRC.                                                                 *
*                                                                                                  *
*    Interfaz simple para IRC que permite toda la interacción mínima con el usuario. Dispone de    *
*    un campo de introducción de la url del servidor, un botón de conexión, una ventana de texto   *
*    para la salida de los mensajes enviados y recibidos, un campo de introducción de mensajes     *
*    y un botón para el envío de los mensajes.                                                     *
*                                                                                                  *
*    Todos los callbacks están completamente resueltos salvo los que tiene que resolver el alumno  *
*    que son el de envío de mensajes y el de conexión. Ambos callbacks llaman a las funciones      *
*       void connectClient(void)                                                                   *
*       void disconnectClient(void)                                                                *
*       void topicProtect(gboolean state)                                                          *
*       void externMsg(gboolean state)                                                             *
*       void secret(gboolean state)                                                                *
*       void guests(gboolean state)                                                                *
*       void privated(gboolean state)                                                              *
*       void moderated(gboolean state)                                                             *
*       void newText (const char *msg)                                                             *
*                                                                                                  *
*    Se proporcionan tres funciones para la impresión de mensajes y errores que el alumno podrá    *
*    utilizar en cualquier punto del programa:                                                     *
*        void publicText(char *user, char *text)                                                   *
*        void privateText(char *user, char *text)                                                  *
*        void errorText(char *errormessage)                                                        *
*        void messageText(char *message)                                                           *
*                                                                                                  *
*    Se proporcionan funciones para obtener algunas variables del entorno gráfico:                 *
*        char * getApodo(void)                                                                     *
*        char * getNombre(void)                                                                    *
*        char * getNombreReal(void)                                                                *
*        char * getServidor(void)                                                                  *
*        int getPuerto(void)                                                                       *
*                                                                                                  *
*    Se proporcionan funciones para cambiar la representación del estado:                          *
*        void setTopicProtect (gboolean state)                                                     *
*        void setExternMsg (gboolean state)                                                        *
*        void setSecret (gboolean state)                                                           *
*        void setGuests (gboolean state)                                                           *
*        void setPrivate (gboolean state)                                                          *
*        void setModerated (gboolean state)                                                        *
*                                                                                                  *
*    Se proporciona una función para presentar ventanas de error:                                  *
*        void errorWindow(char *msg)                                                               *
*                                                                                                  *
***************************************************************************************************/

#include "gui_client.h"
#include "types.h"
#include "log.h"
#include "strings.h"

#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

/* Variables globales */
pthread_t gtk_tid;

#define gtk_threads_enter_if_required() do { if(!pthread_equal(pthread_self(), gtk_tid)) gdk_threads_enter(); } while(0)
#define gtk_threads_leave_if_required() do { if(!pthread_equal(pthread_self(), gtk_tid)) gdk_threads_leave(); } while(0)

GtkWidget *window;
GtkWidget *eApodo, *eNombre, *eNombreR, *eServidor, *ePuerto;
GtkTextIter iter;
GtkTextBuffer *buffer;
GtkWidget *topicB, *externB, *secretB, *guestB, *privateB, *moderatedB;
GtkWidget *btnConnect, *btnDisconnect, *tbInput;
short btnChangeTriggered = FALSE;


gboolean toggleButtonState(GtkToggleButton *togglebutton)
{
    return gtk_toggle_button_get_active(togglebutton);
}

extern int snd_qid;
extern int rcv_sockcomm;
extern struct irc_globdata *ircdata;
extern int serv_sock;
extern struct irc_clientdata *client;

/*******************************************************************************
*  Lee los valores de inicio del chat y los devuelven del tipo que corresponda *
*******************************************************************************/

char *getApodo()
{
    return (char *) gtk_entry_get_text(GTK_ENTRY(eApodo));
}
char *getNombre()
{
    return (char *) gtk_entry_get_text(GTK_ENTRY(eNombre));
}
char *getNombreReal()
{
    return (char *) gtk_entry_get_text(GTK_ENTRY(eNombreR));
}
char *getServidor()
{
    return (char *) gtk_entry_get_text(GTK_ENTRY(eServidor));
}
char *getPuerto()
{
    return (char *) gtk_entry_get_text(GTK_ENTRY(ePuerto));
}

void setApodo(const char *text)
{
    gtk_entry_set_text(GTK_ENTRY(eApodo), text);
}

void setNombre(const char *text)
{
    gtk_entry_set_text(GTK_ENTRY(eNombre), text);
}

void setNombreReal(const char *text)
{
    gtk_entry_set_text(GTK_ENTRY(eNombreR), text);
}

void setServidor(const char *text)
{
    gtk_entry_set_text(GTK_ENTRY(eServidor), text);
}

void setPuerto(const char *text)
{
    gtk_entry_set_text(GTK_ENTRY(ePuerto), text);
}

/*******************************************************************************
*  Presenta una ventana con un mensaje de error                                *
*                                                                              *
*  Parámetros:                                                                 *
*   -msg: Mensaje a presentar                                              *
*  Retorno:                                                                    *
*   -void                                                                  *
*                                                                              *
*******************************************************************************/
void errorWindow(char *msg)
{
    GtkWidget *pError;

    pError = gtk_message_dialog_new (GTK_WINDOW(window), /* Diálogo error envío */
                                     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_CLOSE,
                                    "Error:\n%s", msg);

    gtk_dialog_run(GTK_DIALOG(pError));
    gtk_widget_destroy (pError);
}

int promptWindow(const char* msg)
{
    GtkWidget *diag;
    gint retval;

    diag = gtk_message_dialog_new (GTK_WINDOW(window), 
                                     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_QUESTION,
                                     GTK_BUTTONS_YES_NO,
                                    "%s", msg);

    retval = gtk_dialog_run(GTK_DIALOG(diag));
    gtk_widget_destroy(diag);

    return retval == GTK_RESPONSE_YES;
}

/*******************************************************************************
*  Funciones de uso interno                                                    *
*******************************************************************************/

void connectCB (GtkButton *button, gpointer user_data)
{
    connectClient();
}
void disconnectCB (GtkButton *button, gpointer user_data)
{
    disconnectClient(NULL);
}

void topicProtectCB (GtkToggleButton *togglebutton, gpointer user_data)
{
    if (!btnChangeTriggered)
        topicProtect(toggleButtonState(togglebutton));
    else
        btnChangeTriggered = FALSE;
}

void externMsgCB (GtkToggleButton *togglebutton, gpointer user_data)
{
    if (!btnChangeTriggered)
        externMsg(toggleButtonState(togglebutton));
    else
        btnChangeTriggered = FALSE;
}

void secretCB (GtkToggleButton *togglebutton, gpointer user_data)
{
    if (!btnChangeTriggered)
        secret(toggleButtonState(togglebutton));
    else
        btnChangeTriggered = FALSE;
}

void guestsCB (GtkToggleButton *togglebutton, gpointer user_data)
{
    if (!btnChangeTriggered)
        guests(toggleButtonState(togglebutton));
    else
        btnChangeTriggered = FALSE;
}

void privateCB (GtkToggleButton *togglebutton, gpointer user_data)
{
    if (!btnChangeTriggered)
        privated(toggleButtonState(togglebutton));
    else
        btnChangeTriggered = FALSE;
}

void moderatedCB (GtkToggleButton *togglebutton, gpointer user_data)
{
    if (!btnChangeTriggered)
        moderated(toggleButtonState(togglebutton));
    else
        btnChangeTriggered = FALSE;
}

void readMessageCB (GtkWidget *msg, gpointer user_data)
{
    newText(gtk_entry_get_text(GTK_ENTRY(msg)));
    gtk_entry_set_text(GTK_ENTRY(msg), "");
}

static void gtkEnable(GtkWidget *wd)
{
    gtk_widget_set_sensitive(wd, TRUE);
}

static void gtkDisable(GtkWidget *wd)
{
    gtk_widget_set_sensitive(wd, FALSE);
}

/*******************************************************************************
*  Cambia el estado de todos los check buttons que representan el estado del   *
*  chat                                                                        *
*                                                                              *
*  Parámetros:                                                                 *
*   -state: TRUE ó FALSE                                                   *
*  Retorno:                                                                    *
*   -void                                                                  *
*                                                                              *
*******************************************************************************/

static void _set_btn_state(GtkToggleButton *btn, gboolean state, gboolean enabled)
{
    btnChangeTriggered = TRUE;
    gtk_toggle_button_set_active(btn, state);

    if (enabled)
        gtkEnable(GTK_WIDGET(btn));
    else
        gtkDisable(GTK_WIDGET(btn));
}

void setTopicProtect(gboolean state, gboolean enabled)
{
    _set_btn_state(GTK_TOGGLE_BUTTON(topicB), state, enabled);
}
void setExternMsg(gboolean state, gboolean enabled)
{
    _set_btn_state(GTK_TOGGLE_BUTTON(externB), state, enabled);
}
void setSecret(gboolean state, gboolean enabled)
{
    _set_btn_state(GTK_TOGGLE_BUTTON(secretB), state, enabled);
}
void setGuests(gboolean state, gboolean enabled)
{
    _set_btn_state(GTK_TOGGLE_BUTTON(guestB), state, enabled);
}
void setPrivate(gboolean state, gboolean enabled)
{
    _set_btn_state(GTK_TOGGLE_BUTTON(privateB), state, enabled);
}
void setModerated(gboolean state, gboolean enabled)
{
    _set_btn_state(GTK_TOGGLE_BUTTON(moderatedB), state, enabled);
}

/*******************************************************************************
*  Presenta un mensaje con las características de un mensaje público           *
*                                                                              *
*  Parámetros:                                                                 *
*   -Nombre del usuario que ha enviado el mensaje                          *
*   -Mensaje                                                               *
*  Retorno:                                                                    *
*   -void                                                                  *
*                                                                              *
*******************************************************************************/

void publicText(const char *user, const char *text)
{
    gtk_threads_enter_if_required();
    {
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, user, -1, "blue_fg", "bold", "lmarg",  NULL);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, ": ", -1, "blue_fg", "bold", "lmarg",  NULL);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "italic",  NULL);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "\n", -1, "italic",  NULL);
    }
    gtk_threads_leave_if_required();
}

/*******************************************************************************
*  Presenta un mensaje con las características de un mensaje privado           *
*                                                                              *
*  Parámetros:                                                                 *
*   -Nombre del usuario que ha enviado el mensaje                          *
*   -Mensaje                                                               *
*  Retorno:                                                                    *
*   -void                                                                  *
*                                                                              *
*******************************************************************************/

void privateText(const char *user, const char *text)
{
    gtk_threads_enter_if_required();
    {
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, user, -1, "blue_fg", "bold", "lmarg",  NULL);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, ": ", -1, "blue_fg", "bold", "lmarg",  NULL);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "green_fg",  NULL);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "\n", -1, "green_fg",  NULL);
    }
    gtk_threads_leave_if_required();
}

/*******************************************************************************
*  Presenta un mensaje con las características de un error                     *
*                                                                              *
*  Parámetros:                                                                 *
*   -Mensaje de error                                                      *
*  Retorno:                                                                    *
*   -void                                                                  *
*                                                                              *
*******************************************************************************/

void errorText(const char *errormessage, ...)
{
    gtk_threads_enter_if_required();
    {
        char fmt_message[500];
        va_list ap;
        va_start(ap, errormessage);
        vsnprintf(fmt_message, 500, errormessage, ap);
        va_end(ap);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "Error: ", -1, "magenta_fg", "black_bg", "italic", "bold", "lmarg",  NULL);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, fmt_message, -1, "magenta_fg", "black_bg", "italic", "bold", "lmarg",  NULL);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "\n", -1, "magenta_fg",  NULL);
    }
    gtk_threads_leave_if_required();
}

/*******************************************************************************
*  Presenta un mensaje cualquiera del programa al usuario                      *
*                                                                              *
*  Parámetros:                                                                 *
*   -Mensaje                                                               *
*  Retorno:                                                                    *
*   -void                                                                  *
*                                                                              *
*******************************************************************************/

void messageText(const char *message, ...)
{
    gtk_threads_enter_if_required();
    {

        char fmt_message[500];
        va_list ap;
        va_start(ap, message);
        vsnprintf(fmt_message, 500, message, ap);
        va_end(ap);

        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, fmt_message, -1, "magenta_fg", "italic", "bold", "lmarg",  NULL);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "\n", -1, "magenta_fg",  NULL);
    }
    gtk_threads_leave_if_required();

}

void noticeText(const char *message, ...)
{
    gtk_threads_enter_if_required();
    {
        char fmt_message[500];
        va_list ap;
        va_start(ap, message);
        vsnprintf(fmt_message, 500, message, ap);
        va_end(ap);

        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, fmt_message, -1, "blue_fg", "italic", "lmarg",  NULL);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "\n", -1, "blue_fg",  NULL);
    }
    gtk_threads_leave_if_required();
}

void actionText(const char *username, const char *message, ...)
{
    gtk_threads_enter_if_required();
    {
        char fmt_message[500];
        va_list ap;
        va_start(ap, message);
        vsnprintf(fmt_message, 500, message, ap);
        va_end(ap);

        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, username, -1, "green", "bold", "lmarg",  NULL);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, fmt_message, -1, "green", "lmarg",  NULL);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "\n", -1, "green",  NULL);
    }
    gtk_threads_leave_if_required();
}

void setUserConnectionState(const gboolean isConnected)
{
    if (isConnected)
    {
        gtkDisable(btnConnect);
        gtkEnable(btnDisconnect);
    }
    else
    {
        gtkEnable(btnConnect);
        gtkDisable(btnDisconnect);
    }
}


/*******************************************************************************
*  Realiza un scrolling cada vez que se presenta un mensaje                    *
*                                                                              *
*  Parámetros y retorno según callbacks de gnome                               *
*                                                                              *
*******************************************************************************/

void scrolling(GtkWidget *widget, gpointer data)
{
    gtk_threads_enter_if_required();
    {
        GtkAdjustment *adjustment;

        adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(widget));
        adjustment->value = adjustment->upper;
        gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(widget), adjustment);
    }
    gtk_threads_leave_if_required();
}

void ConnectArea(GtkWidget *vbox)
{
    GtkWidget *table;
    GtkWidget *hb1, *hb2, *hb3, *hb4, *hb5, *hb6;
    GtkWidget *l1, *l2, *l3, *l4, *l5;
    GtkWidget *frm;

    frm = gtk_frame_new("Conexión");
    gtk_box_pack_start(GTK_BOX(vbox), frm, FALSE, FALSE, 2);
    table = gtk_table_new(1, 6, FALSE);
    gtk_container_add(GTK_CONTAINER(frm), table);

    hb1 = gtk_hbox_new(FALSE, 2);
    hb2 = gtk_hbox_new(FALSE, 2);
    hb3 = gtk_hbox_new(FALSE, 2);
    hb4 = gtk_hbox_new(FALSE, 2);
    hb5 = gtk_hbox_new(FALSE, 2);
    hb6 = gtk_hbox_new(FALSE, 2);
    gtk_table_attach(GTK_TABLE(table), hb1, 0, 1, 0, 1, GTK_FILL | GTK_SHRINK, GTK_SHRINK, 2, 2);
    gtk_table_attach(GTK_TABLE(table), hb2, 0, 1, 1, 2, GTK_FILL | GTK_SHRINK, GTK_SHRINK, 2, 2);
    gtk_table_attach(GTK_TABLE(table), hb3, 0, 1, 2, 3, GTK_FILL | GTK_SHRINK, GTK_SHRINK, 2, 2);
    gtk_table_attach(GTK_TABLE(table), hb4, 0, 1, 3, 4, GTK_FILL | GTK_SHRINK, GTK_SHRINK, 2, 2);
    gtk_table_attach(GTK_TABLE(table), hb5, 0, 1, 4, 5, GTK_FILL | GTK_SHRINK, GTK_SHRINK, 2, 2);
    gtk_table_attach(GTK_TABLE(table), hb6, 0, 1, 5, 6, GTK_FILL | GTK_SHRINK, GTK_SHRINK, 2, 2);
    l1 = gtk_label_new("Apodo");
    l2 = gtk_label_new("Nombre");
    l3 = gtk_label_new("Nombre real");
    l4 = gtk_label_new("Servidor");
    l5 = gtk_label_new("Puerto");
    eApodo    = gtk_entry_new();
    eNombre   = gtk_entry_new();
    eNombreR  = gtk_entry_new();
    eServidor = gtk_entry_new();
    ePuerto   = gtk_entry_new();
    btnConnect = gtk_button_new_with_mnemonic("_Conectar");
    btnDisconnect = gtk_button_new_with_mnemonic("_Desconectar");

    gtk_box_pack_start(GTK_BOX(hb1), l1 , FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hb2), l2 , FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hb3), l3 , FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hb4), l4 , FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hb5), l5 , FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hb6), btnConnect, TRUE , TRUE , 2);

    gtk_box_pack_end(GTK_BOX(hb1), eApodo   , FALSE, FALSE, 2);
    gtk_box_pack_end(GTK_BOX(hb2), eNombre  , FALSE, FALSE, 2);
    gtk_box_pack_end(GTK_BOX(hb3), eNombreR , FALSE, FALSE, 2);
    gtk_box_pack_end(GTK_BOX(hb4), eServidor, FALSE, FALSE, 2);
    gtk_box_pack_end(GTK_BOX(hb5), ePuerto  , FALSE, FALSE, 2);
    gtk_box_pack_end(GTK_BOX(hb6), btnDisconnect      , TRUE , TRUE , 2);

    g_signal_connect(G_OBJECT(btnConnect), "clicked", G_CALLBACK(connectCB), NULL);
    g_signal_connect(G_OBJECT(btnDisconnect), "clicked", G_CALLBACK(disconnectCB), NULL);

}

void StateArea(GtkWidget *vbox)
{
    GtkWidget *vboxi;
    GtkWidget *frm;

    frm = gtk_frame_new("Estado");
    gtk_box_pack_start(GTK_BOX(vbox), frm, FALSE, FALSE, 2);
    vboxi = gtk_vbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(frm), vboxi);

    topicB    = gtk_check_button_new_with_mnemonic("_Protección de tópico");
    externB   = gtk_check_button_new_with_mnemonic("Mensajes _externos");
    secretB   = gtk_check_button_new_with_mnemonic("_Secreto");
    guestB    = gtk_check_button_new_with_mnemonic("Sólo _invitados");
    privateB  = gtk_check_button_new_with_mnemonic("Pri_vado");
    moderatedB    = gtk_check_button_new_with_mnemonic("_Moderado");

    gtk_box_pack_start(GTK_BOX(vboxi), topicB , TRUE , TRUE , 2);
    gtk_box_pack_start(GTK_BOX(vboxi), externB    , TRUE , TRUE , 2);
    gtk_box_pack_start(GTK_BOX(vboxi), secretB    , TRUE , TRUE , 2);
    gtk_box_pack_start(GTK_BOX(vboxi), guestB , TRUE , TRUE , 2);
    gtk_box_pack_start(GTK_BOX(vboxi), privateB   , TRUE , TRUE , 2);
    gtk_box_pack_start(GTK_BOX(vboxi), moderatedB , TRUE , TRUE , 2);

    g_signal_connect(G_OBJECT(topicB) , "toggled", G_CALLBACK(topicProtectCB), NULL);
    g_signal_connect(G_OBJECT(externB)    , "toggled", G_CALLBACK(externMsgCB), NULL);
    g_signal_connect(G_OBJECT(secretB)    , "toggled", G_CALLBACK(secretCB), NULL);
    g_signal_connect(G_OBJECT(guestB) , "toggled", G_CALLBACK(guestsCB), NULL);
    g_signal_connect(G_OBJECT(privateB)   , "toggled", G_CALLBACK(privateCB), NULL);
    g_signal_connect(G_OBJECT(moderatedB) , "toggled", G_CALLBACK(moderatedCB), NULL);
}


void ChatArea(GtkWidget *vbox)
{
    GtkWidget *hbox;
    GtkWidget *frame, *scroll, *view, *label;
    GtkAdjustment *adjustment; /* Uso interno de ajuste de scroll */

    frame  = gtk_frame_new("Chat");
    scroll = gtk_scrolled_window_new(NULL, NULL);
    view   = gtk_text_view_new();
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    label  = gtk_label_new("Mensaje");
    hbox   = gtk_hbox_new(FALSE, 2),
    tbInput    = gtk_entry_new();

    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 2);
    gtk_container_add(GTK_CONTAINER(frame), scroll);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll), view);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
    gtk_box_pack_end(GTK_BOX(hbox), tbInput, FALSE, FALSE, 2);

    gtk_widget_set_size_request(tbInput, 600, -1);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD_CHAR);
    gtk_widget_set_size_request(scroll, 600, 330);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(scroll), GTK_CORNER_BOTTOM_LEFT);
    adjustment = (GtkAdjustment *) gtk_adjustment_new(0., 0., 396., 18., 183., 396.);
    gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW(scroll), adjustment);

    gtk_text_buffer_create_tag(buffer, "lmarg", "left_margin", 5, NULL);
    gtk_text_buffer_create_tag(buffer, "red_fg", "foreground", "red", NULL);
    gtk_text_buffer_create_tag(buffer, "blue_fg", "foreground", "blue", NULL);
    gtk_text_buffer_create_tag(buffer, "magenta_fg", "foreground", "#FF00FF", NULL);
    gtk_text_buffer_create_tag(buffer, "black_bg", "background", "black", NULL);
    gtk_text_buffer_create_tag(buffer, "green_fg", "foreground", "#00BB00", NULL);
    gtk_text_buffer_create_tag(buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
    gtk_text_buffer_create_tag(buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);

    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

    g_signal_connect(G_OBJECT(tbInput), "activate", G_CALLBACK(readMessageCB), NULL);
    g_signal_connect(G_OBJECT(scroll), "size-allocate", G_CALLBACK(scrolling), NULL);
}


int spawn_thread_gui(pthread_t *gui_thread, int argc, const char **argv)
{
    struct gui_data *thdata = malloc(sizeof(struct gui_thdata));

    if (pthread_create(gui_thread, NULL, thread_gui, thdata))
    {
        slog(LOG_CRIT, "Error creando el hilo de la interfaz: %s", strerror(errno));
        return ERR;
    }
    else
    {
        slog(LOG_INFO, "Hilo de interfaz creado");
    }

    return OK;
}

int saveUserSettings(const char *nick, const char *name, const char *realname)
{
    FILE *f = fopen(USERSETTINGS_FILE, "w");

    if (!f)
        return ERR_SYS;

    fprintf(f, "%s\n%s\n%s\n", nick, name, realname);

    fclose(f);

    return OK;
}

void tryReadUserSettings()
{
    FILE *f = fopen(USERSETTINGS_FILE, "r");
    char nick[100], name[100], realname[100];

    if (!f)
        return;

    if (fgets(nick, 100, f) && fgets(name, 100, f) && fgets(realname, 100, f))
    {
        strip_end(nick);
        strip_end(name);
        strip_end(realname);
        setApodo(nick);
        setNombre(name);
        setNombreReal(realname);
    }

    fclose(f);
}

void *thread_gui(void *data)
{
    struct gui_thdata *thdata = (struct gui_thdata *)data;

    GtkWidget *hboxg, *vbox1, *vbox2;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    g_type_init (); /* Necesario para tener funcionalidad de hilos */
#pragma GCC diagnostic pop

    gtk_tid = pthread_self();

    gdk_threads_init ();
    gdk_threads_enter ();
    gtk_init(&(thdata->argc), NULL); /* Inicia gnome */

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL); /* Ventana principal */

    gtk_window_set_title(GTK_WINDOW(window), "IRC Chat"); /* Título ventana principal */
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 350); /* Tamaño ventana principal */
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE); /* Posición ventana principal */

    /* Estructura global */
    hboxg = gtk_hbox_new(FALSE, 5);
    vbox1 = gtk_vbox_new(FALSE, 5);
    vbox2 = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(window), hboxg);
    gtk_box_pack_start(GTK_BOX(hboxg), vbox1, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(hboxg), vbox2, FALSE, FALSE, 1);
    ConnectArea(vbox1);
    StateArea(vbox1);
    ChatArea(vbox2);

    setUserConnectionState(FALSE);
    tryReadUserSettings();
    disableChanMods();

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window); /* Presentación de las ventanas */
    gdk_threads_leave (); /* Salida de hilos */

    gdk_threads_enter();
    gtk_main(); /* Administración de la interacción */
    gdk_threads_leave();

    raise(SIGTERM); /* Cuando acaba la interfaz, salimos del programa */

    return 0;
}


void enableChanMods()
{
    setTopicProtect(FALSE, TRUE);
    setExternMsg(FALSE, TRUE);
    setSecret(FALSE, TRUE);
    setGuests(FALSE, TRUE);
    setPrivate(FALSE, TRUE);
    setModerated(FALSE, TRUE);
}

void disableChanMods()
{
    setTopicProtect(FALSE, FALSE);
    setExternMsg(FALSE, FALSE);
    setSecret(FALSE, FALSE);
    setGuests(FALSE, FALSE);
    setPrivate(FALSE, FALSE);
    setModerated(FALSE, FALSE);
}
