#include "element.h"

using namespace cpplab3v13;

int main() {
	int rc;
	int(*function_ptr[])(element&) = { nullptr, d_add_conn, d_del_conn,
									   d_show_all, d_disconnect_conn,
									   d_connect_conn,d_change_all_states,
									   d_print_conn_state, d_set_conn_state };


	element elem;
	while ((rc = dialog())) {
		if (!function_ptr[rc](elem)) break;
	}
	return 0;
}