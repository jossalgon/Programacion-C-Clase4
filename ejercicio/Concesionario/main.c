#include "concesionario.h"
#include <gtk/gtk.h>
#include <string.h>

struct concesionario *con;
int id;
int selected;

static void combo_selected(GtkWidget *combo, gpointer data)
{
	selected = gtk_combo_box_get_active((GtkComboBox *)combo);
}

static void funcion_quit(GtkButton *boton, gpointer data)
{
	curso_concesionario_free(con);
	gtk_main_quit();
}

static void funcion_save(char *filename)
{
	escribeFichero(filename, con);
}

static void funcion_delete(GtkWidget *widget, gpointer data)
{
	curso_concesionario_attr_unset_coche(con, selected);
	gtk_combo_box_text_remove((GtkComboBoxText *)data, selected);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data), 0);
	selected = 0;
}

static void funcion_load(char *fsel)
{
	curso_concesionario_free(con);
	con = curso_concesionario_alloc();
	leeFichero(fsel, con);
	id = curso_concesionario_attr_get_u32(con,
		CURSO_CONCESIONARIO_ATTR_NUM_COCHES);
	curso_concesionario_attr_set_str(con, CURSO_CONCESIONARIO_ATTR_DUENO,
					 "Jose");
}

int leeFichero(char *file, struct concesionario * con)
{
	FILE *fichero;
	char line[256];
	struct coche *c;

	fichero = fopen(file, "r");

	if (fichero == NULL) {
		printf("Error (NO ABIERTO)\n");
		return -1;
	}

	while (fgets(line, sizeof(line), fichero)) {
		char *p;
		int i = 0;
		c = curso_coche_alloc();

		p = strtok(line, ",");
		while (p != NULL) {
			switch (i) {
			case 0:
				curso_coche_attr_set_str(c, CURSO_COCHE_ATTR_MATRICULA, p);
				break;
			case 1:
				curso_coche_attr_set_str(c, CURSO_COCHE_ATTR_MARCA, p);
				break;
			case 2:
				curso_coche_attr_set_u32(c, CURSO_COCHE_ATTR_ID, atoi(p));
				break;
			default:
				break;
			}

			i++;
			p = strtok(NULL, ",");
		}

		curso_concesionario_attr_set_coche(con,
					CURSO_CONCESIONARIO_ATTR_COCHE, c);
	}

	if (fclose(fichero)) {
		printf("Error: fichero NO CERRADO\n");
		return -1;
	}

	return 0;
}

int escribeFichero(char *file, struct concesionario * con) {
	FILE *fichero;
	char line[256];
	struct coche *c;
	int i;

	fichero = fopen(file, "w");

	if (fichero == NULL) {
		printf("Error (NO ABIERTO)\n");
		return -1;
	}

	for (i = 0; i< curso_concesionario_attr_get_u32(con,
				CURSO_CONCESIONARIO_ATTR_NUM_COCHES); i++) {

		c = curso_concesionario_attr_get_coche(con,
			CURSO_CONCESIONARIO_ATTR_COCHE, i);
		char id[10];
		sprintf(id, "%d", curso_coche_attr_get_u32(c, CURSO_COCHE_ATTR_ID));

		fputs(curso_coche_attr_get_str(c, CURSO_COCHE_ATTR_MATRICULA), fichero);
		fputc(',', fichero);
		fputs(curso_coche_attr_get_str(c, CURSO_COCHE_ATTR_MARCA), fichero);
		fputc(',', fichero);
		fputs(id, fichero);
		fputc('\n', fichero);

	}

	if (fclose(fichero)) {
		printf("Error: fichero NO CERRADO\n");
		return -1;
	}

	return 0;
}

static void add_car(GtkButton *widget, gpointer data)
{
	GtkWidget **entry;
	struct coche *c;
	GtkWidget *entry_ptr_marca;
	GtkWidget *entry_ptr_matricula;
	const gchar *matricula, *marca;

	entry = (GtkWidget **)data;
	entry_ptr_marca = entry[0];
	entry_ptr_matricula = entry[1];

	marca = gtk_entry_get_text((GtkEntry *)entry_ptr_marca);
	matricula = gtk_entry_get_text((GtkEntry *)entry_ptr_matricula);

	c = curso_coche_alloc();
	if (c == NULL)
		return;

	curso_coche_attr_set_u32(c, CURSO_COCHE_ATTR_ID, id++);
	curso_coche_attr_set_str(c, CURSO_COCHE_ATTR_MATRICULA, matricula);
	curso_coche_attr_set_str(c, CURSO_COCHE_ATTR_MARCA, marca);

	curso_concesionario_attr_set_coche(con, CURSO_CONCESIONARIO_ATTR_COCHE,
					   c);
}

static void show_info_window(GtkButton *widget, gpointer data)
{
	GtkWidget *window;
	GtkWidget *box;
	static GtkWidget *ginfo;
	char info[4000];
	GtkWidget *image;

	curso_concesionario_snprintf(info, sizeof(info), con);
	ginfo = gtk_label_new(info);
	image = gtk_image_new_from_file ("image2.jpg");

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	box = gtk_vbox_new(TRUE, 2);
	gtk_window_set_title((GtkWindow *)window, "Información");

	gtk_container_add((GtkContainer *)window, box);
	gtk_box_pack_start((GtkBox *)box, image, TRUE, TRUE, 0);
	gtk_box_pack_start((GtkBox *)box, ginfo, TRUE, TRUE, 2);
	gtk_widget_show_all(window);
}

static void show_delete_window(GtkButton *widget, gpointer data)
{
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *box;
	GtkWidget *combo;
	GtkWidget *image;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	box = gtk_vbox_new(FALSE, 2);
	combo = gtk_combo_box_text_new();
	image = gtk_image_new_from_file("image2.jpg");

	gtk_window_set_title((GtkWindow *)window, "Eliminar coche");
	gtk_window_set_default_size(GTK_WINDOW(window), 250, 250);
	button = gtk_button_new_with_label("Eliminar");

	// Añade coches al combo
	int i;
	struct coche *c;
	c = curso_coche_alloc();
	int numCoches = curso_concesionario_attr_get_u32(con,
		CURSO_CONCESIONARIO_ATTR_NUM_COCHES);
	char infoCoche[4000];
	for (i = 0; i < numCoches; i++) {
		c = curso_concesionario_attr_get_coche(con,
			CURSO_CONCESIONARIO_ATTR_COCHE, i);

		curso_coche_snprintf(infoCoche, sizeof(infoCoche), c);
		gtk_combo_box_text_append_text((GtkComboBoxText *)combo,
		(gchar *)infoCoche);
	}

	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
	selected = 0;
	gtk_widget_set_size_request(combo, -1, 50);
	gtk_widget_set_size_request(button, -1, 100);

	gtk_box_pack_start((GtkBox *)box, image, TRUE, TRUE, 0);
	gtk_box_pack_start((GtkBox *)box, combo, TRUE, TRUE, 2);
	gtk_box_pack_start((GtkBox *)box, button, TRUE, TRUE, 2);

	gtk_container_add((GtkContainer *)window, box);
	gtk_widget_show_all(window);

	g_signal_connect((GObject *)combo, "changed",
			 (GCallback)combo_selected, (gpointer) combo);
	g_signal_connect((GObject *)button, "clicked",
			 (GCallback)funcion_delete, (gpointer) combo);

}

void show_load_window(GtkWidget *button, gpointer window)
{
	GtkWidget *dialog;
	gint res;


	dialog = gtk_file_chooser_dialog_new("Selecciona un archivo",
		GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL, NULL);

	res = gtk_dialog_run(GTK_DIALOG (dialog));
	if (res == GTK_RESPONSE_OK) {
		char *filename;
		filename = gtk_file_chooser_get_filename(
						GTK_FILE_CHOOSER(dialog));
		funcion_load(filename);
		g_free(filename);
	}

	gtk_widget_destroy(dialog);
}

void show_save_window(GtkWidget *button, gpointer window)
{
	GtkWidget *dialog;
	GtkFileChooser *chooser;
	gint res;

	dialog = gtk_file_chooser_dialog_new("Guardar archivo",
		GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL, NULL);

	chooser = GTK_FILE_CHOOSER (dialog);
	res = gtk_dialog_run (GTK_DIALOG (dialog));

	if (res == GTK_RESPONSE_OK) {
		char *filename;
		filename = gtk_file_chooser_get_filename(chooser);
		funcion_save(filename);
		g_free(filename);
	}

	gtk_widget_destroy(dialog);
}

void show_about_window(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog = gtk_about_dialog_new();
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), "Concesionario");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "0.1");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog),
		"(c) Jose Luis Salazar Gonzalez");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog),
		"Concesionario es una herramienta simple para gestionar coches");
	gtk_dialog_run(GTK_DIALOG (dialog));
	gtk_widget_destroy(dialog);
}

int main (int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *button1;
	GtkWidget *button2;
	GtkWidget *button3;
	GtkWidget *image;
	GtkWidget *vbox;
	GtkWidget *hbox;

	GtkWidget *menubar;
	GtkWidget *filemenu;
	GtkWidget *file;
	GtkWidget *save;
	GtkWidget *load;
	GtkWidget *quit;
	GtkWidget *editmenu;
	GtkWidget *edit;
	GtkWidget *delete;
	GtkWidget *viewmenu;
	GtkWidget *view;
	GtkWidget *infoCon;
	GtkWidget *helpmenu;
	GtkWidget *about;
	GtkWidget *help;

	GtkWidget *entry1, *entry2;
	GtkWidget **entry;
	GtkWidget *marca;
	GtkWidget *matricula;
	GtkWidget *hseparator;

	gtk_init(&argc, &argv);

	entry = calloc(1, 2 * sizeof(GtkWidget));

	entry1 = gtk_entry_new();
	entry2 = gtk_entry_new();

	entry[0] = entry1;
	entry[1] = entry2;

	marca = gtk_label_new("Marca:");
	matricula = gtk_label_new("Matrícula:");
	hseparator = gtk_hseparator_new();

	con = curso_concesionario_alloc();
	if (con == NULL)
		return -1;

	curso_concesionario_attr_set_str(con, CURSO_CONCESIONARIO_ATTR_DUENO,
					 "Jose");

	menubar = gtk_menu_bar_new();
	filemenu = gtk_menu_new();
	editmenu = gtk_menu_new();
	viewmenu = gtk_menu_new();
	helpmenu = gtk_menu_new();


	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);

	button1 = gtk_button_new_with_label("Añadir coche");
	button2 = gtk_button_new_with_label("Información");
	button3 = gtk_button_new_with_label("Eliminar coche");
	image = gtk_image_new_from_file ("image.jpg");
	vbox = gtk_vbox_new(FALSE,0);
	hbox = gtk_hbox_new(FALSE,0);
	file = gtk_menu_item_new_with_label("Archivo");
	save = gtk_menu_item_new_with_label("Guardar");
	load = gtk_menu_item_new_with_label("Cargar");
	quit = gtk_menu_item_new_with_label("Cerrar");
	edit = gtk_menu_item_new_with_label("Editar");
	delete = gtk_menu_item_new_with_label("Eliminar coche");
	view = gtk_menu_item_new_with_label("Vista");
	infoCon = gtk_menu_item_new_with_label("Información");
	about = gtk_menu_item_new_with_label("Acerca de");
	help = gtk_menu_item_new_with_label("Ayuda");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), save);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), load);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit), editmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), delete);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), edit);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(view), viewmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(viewmenu), infoCon);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), view);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), helpmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), about);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help);
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 3);

	gtk_window_set_title((GtkWindow *)window, "Concesionario");

	gtk_box_pack_start((GtkBox *)vbox, image, TRUE, TRUE, 0);
	gtk_box_pack_start((GtkBox *)vbox, hbox, TRUE, TRUE, 0);
	gtk_box_pack_start((GtkBox *)vbox, button1, TRUE, TRUE, 2);
	gtk_box_pack_start((GtkBox *)vbox, hseparator, FALSE, TRUE, 10);
	gtk_box_pack_start((GtkBox *)vbox, button2, TRUE, TRUE, 2);
	gtk_box_pack_start((GtkBox *)vbox, button3, TRUE, TRUE, 2);

	gtk_box_pack_start ((GtkBox *)hbox, marca, TRUE, TRUE, 2);
	gtk_box_pack_start ((GtkBox *)hbox, entry1, TRUE, TRUE, 2);
	gtk_box_pack_start ((GtkBox *)hbox, matricula, TRUE, TRUE, 2);
	gtk_box_pack_start ((GtkBox *)hbox, entry2, TRUE, TRUE, 2);

	gtk_container_add((GtkContainer *)window, vbox);

	g_signal_connect((GObject *)button1, "clicked", (GCallback)add_car,
			 (gpointer)entry);
	g_signal_connect((GObject *)button2, "clicked",
			 (GCallback)show_info_window, NULL);
	g_signal_connect((GObject *)button3, "clicked",
			 (GCallback)show_delete_window, NULL);
	g_signal_connect((GObject *)window, "delete-event",
			 (GCallback)funcion_quit, NULL);
	g_signal_connect((GObject *)save, "activate",
			 (GCallback)show_save_window, NULL);
	g_signal_connect((GObject *)load, "activate",
			 (GCallback)show_load_window, NULL);
	g_signal_connect((GObject *)quit, "activate",
			 (GCallback)funcion_quit, NULL);
	g_signal_connect((GObject *)delete, "activate",
			 (GCallback)show_delete_window, NULL);
	g_signal_connect((GObject *)infoCon, "activate",
			 (GCallback)show_info_window, NULL);
	g_signal_connect((GObject *)about, "activate",
			 (GCallback)show_about_window, NULL);

	gtk_widget_show_all(window);

	gtk_main();
	return 0;
}
