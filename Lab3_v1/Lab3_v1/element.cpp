#include "element.h"

namespace cpplab3v13 {

	const char *messages[] = { "0. Quit                ",
							  "1. Add connection",
							  "2. Delete connection",
							  "3. Print info",
							  "4. Disconnect something",
							  "5. Connect something",
							  "6. Total state change",
							  "7. Get connection state",
							  "8. Set connection state" };

	const int NMsgs = sizeof(messages) / sizeof(messages[0]);
	
	element::element(int in, int out) {
		this->conns = 0;
		for (int i = 0; i < (in + out) && i < connections_max; ++i) {
			connection con;
			con.type = i < in ? IN : OUT;
			add_conn(con);
		}
	}

	element::element(connection *arr, int sum) {
		this->conns = 0;
		for (int i = 0; i < sum; ++i) {
			if (arr[i].type == IM) arr[i].type = IN;
			for (int j = 0; j < 3; ++j) {
				arr[i].sockets[j] = -1;
			}
			add_conn(arr[i]);
		}
	}

	element::element(const connection cn) {
		conns = 1;
		cs[0] = cn;
	}


	element &element::add_conn(const connection newcomer) {
		if (this->conns >= connections_max)
			throw std::runtime_error("Too many connections to add more!");
		for (int i = 0; i < connections_max; ++i) {
			if (cs[i].type == IM) {
				cs[i] = newcomer;
				this->conns++;
				break;
			}
		}
		return *this;
	}

	element &element::disconnect_conn(const int which) {
		if (which < 0 || which >= connections_max)
			throw std::runtime_error("invalid connection index");
		if (cs[which].type == IM)
			throw std::runtime_error("there is no such connection");

		if (cs[which].type == IN) {
			if (cs[which].sockets[0] != -1) {
				for (int i = 0; i < 3; ++i) {
					if (cs[cs[which].sockets[0]].sockets[i] == which)
						cs[cs[which].sockets[0]].sockets[i] = -1;
				}
				cs[which].sockets[0] = -1;
				bool lonely = true;
				for (int j = 0; j < 3; ++j) {
					if (cs[cs[which].sockets[0]].sockets[j] != -1) {
						lonely = false;
						break;
					}
				}
				if (lonely) cs[cs[which].sockets[0]].condition = X;
			}
		}
		else {
			for (int i = 0; i < 3; ++i) { //OUT-type has 3 available sockets
				if (cs[which].sockets[i] != -1) {   // find a connected socket
					for (int j = 0; j < connections_max; ++j) { //find where to
						cs[cs[which].sockets[i]].sockets[0] = -1; //OUT can only be connected to IN
						cs[cs[which].sockets[i]].condition = X;//and IN has only one available socket
					}
				}
			}
			for (int i = 0; i < 3; ++i) {
				cs[which].sockets[i] = -1;
			}
		}
		cs[which].condition = X;
		return *this;
	}

	element &element::delete_conn(const int which) {
		if (which < 0 || which >= connections_max)
			throw std::runtime_error("invalid connection index");
		if (cs[which].type == IM)
			throw std::runtime_error("there is no such connection");
		for (int i = 0; i < conns; ++i) {
			for (int j = 0; j < 3; ++j) {
				if (cs[i].sockets[j] == which)
					cs[i].sockets[j] = -1;
			}
		}
		cs[which] = cs[--conns];
		cs[conns].type = IM;
		cs[conns].condition = X;
		for (int i = 0; i < 3; ++i) {
			if (cs[which].sockets[i] != -1) {
				for (int j = 0; j < 3; ++j) {
					if (cs[cs[which].sockets[i]].sockets[j] == conns) {
						cs[cs[which].sockets[i]].sockets[j] = which;
					}
				}

			}
		}
		return *this;
	}

	conditions& element::set_state(const int index) {
		if (index < 0 || index >= connections_max)
			throw std::runtime_error("invalid connection index");
		if (cs[index].type == IM)
			throw std::runtime_error("there is no such connection");
		return cs[index].condition;
	}

	conditions element::get_state(const int index) const { //const connection
		if (index < 0 || index >= connections_max)
			throw std::runtime_error("invalid connection index");
		if (cs[index].type == IM)
			throw std::runtime_error("there is no such connection");
		return cs[index].condition;
	}

	element &element::connect_conns(int which, int whereto) {
		if (which < 0 || which >= connections_max)
			throw std::runtime_error("invalid connection index");
		if (cs[which].type == IM)
			throw std::runtime_error("there is no such connection");
		if (whereto < 0 || whereto >= connections_max)
			throw std::runtime_error("invalid connection index");
		if (cs[whereto].type == IM)
			throw std::runtime_error("there is no such connection");
		if (cs[whereto].type == cs[which].type)
			throw std::runtime_error("you can\'t connect same types");

		if (cs[which].type == OUT) {
			int h = which;
			which = whereto;
			whereto = h;
		}

		for (int i = 0; i < 3; ++i) {
			if (cs[whereto].sockets[i] == -1) {
				break;
			}
			if (i == 2)
				throw std::runtime_error("no place to plug in in target");
		}

		if (cs[which].sockets[0] != -1)
			throw std::runtime_error("this connection is busy! disconnect it first!");
		else
			cs[which].sockets[0] = whereto;

		for (int i = 0; i < 3; ++i) {
			if (cs[whereto].sockets[i] == -1) {
				cs[whereto].sockets[i] = which;
				break;
			}
		}

		return *this;
	}

	element &element::add(const element& elem) {
		for (int i = 0, j = 0; i < connections_max; ++i) {
			if (cs[i].type == IM) {
				if (j >= elem.conns) break;   // end of conns in elem
				cs[i] = elem.cs[j];
				++conns;
				++j;
			}
		}
		return *this;
	}

	void print (const element& elem) {
		std::cout << "info about all existing connections:" << std::endl;

		for (int i = 0; i < elem.conns; ++i) {
			std::cout << "connection #" << (i + 1) << ":" << std::endl << "Condition: ";
			switch (elem.cs[i].condition) {
			case 0:
				std::cout << "LOW; ";
				break;
			case 1:
				std::cout << "HIGH; ";
				break;
			default:
				std::cout << "NOT DEFINED; ";
				break;
			}
			std::cout << "type: ";
			switch (elem.cs[i].type) {
			case 0:
				std::cout << "INPUT; ";
				break;
			case 1:
				std::cout << "OUTPUT; ";
				break;
			default:
				std::cout << "IMAGINARY; ";
				break;
			}
			std::cout << "connected to elements:";
			for (int j = 0; j < 3; ++j) {
				if (elem.cs[i].sockets[j] != -1) {
					std::cout << " " << (elem.cs[i].sockets[j] + 1);
				}
			}
			std::cout << std::endl << std::endl;
		}
	}

	std::istream &reorg(std::istream& s, element& elem) {
		int c, rc;
		for (int i = 0; i < elem.conns; ++i) {
			bool lonely = true;
			for (int j = 0; j < 3; ++j) {
				if (elem.cs[i].sockets[j] != -1) {
					lonely = false;
					break;
				}
			}
			if (lonely) continue;

			std::cout << "Please, enter the condition(1 for high signal level, "
				<< "0 for low signal level, anything else for X)"
				<< std::endl << "of connection #" << (i + 1)
				<< " of type " << (elem.cs[i].type == IN ? "INPUT:" : "OUTPUT:")
				<< std::endl;

			do {
				rc = input_number(c, s);
				if (rc == 1) break;
				if (!rc) return s;  // eof
				std::cout << "incorrect input, please, try again:";
			} while (rc < 0);

			switch (c) {
			case 0:
				elem.cs[i].set_cond(0);
				break;
			case 1:
				elem.cs[i].set_cond(1);
				break;
			default:
				elem.cs[i].set_cond(2);
				break;
			}
		}
		return s;
	}

	connection element::get_conn(int id) const {
		if (id < 0 || id >= connections_max)
			throw std::runtime_error("invalid connection index");
		return cs[id];
	}

	void signal_handler(int signal) {
		if (signal == SIGINT) {
			std::cerr << "SIGINT received\n";
		}
		else {
			std::cerr << "Unexpected signal: " << signal << "\n";
		}
		std::_Exit(EXIT_SUCCESS);
	}

	int dialog() {
		char *report = (char*)"";
		int rc, i, n;
		do {
			std::cout << report << std::endl;
			report = (char*)"You are wrong. Please, try again.";
			for (i = 0; i < NMsgs; ++i) {  //print list of alternatives
				std::cout << messages[i] << std::endl;
			}
			printf("Make your choice: ~ ");
			n = input_number(rc, std::cin);  //enter number of alternative
			if (!n) rc = 0;  //EOF == end of the program
		} while (rc < 0 || rc >= NMsgs);
		return rc;
	}

	int d_add_conn(element& elem) {
		int rc, b;
		connection con;
	

		std::cout << "Input connection\'s type(1 for in, else for out):"
			<< std::endl;

		do {
			rc = input_number(b, std::cin);
			if (rc == 1) break;
			if (!rc) return 0;
			std::cout << "Incorrect input, please, try again:";
		} while (rc < 0);

		con.type = (b == 1 ? IN : OUT);
		element elem1(con);

		try {
			elem.add(elem1);
		}
		catch (std::runtime_error &rt) {
			std::cout << rt.what() << std::endl;
		}
		return 1;
	}


	int d_del_conn(element& elem) {
		int rc, b;

		std::cout << "Input connection\'s current id to delete:" << std::endl;

		do {
			rc = input_number(b, std::cin);
			if (rc == 1) break;
			if (!rc) return 0;
			std::cout << "Incorrect input, please, try again:";
		} while (rc < 0);

		try {
			elem.delete_conn(b - 1);
		}
		catch (std::runtime_error &rt) {
			std::cout << rt.what() << std::endl;
		}
		return 1;
	}

	int d_show_all(element& elem) {
		print(elem);

		return 1;
	}

	int d_disconnect_conn(element& elem) {
		int rc, a;

		std::cout << "Input connection\'s current id to disconnect:" << std::endl;
		do {
			rc = input_number(a, std::cin);
			if (rc == 1) break;
			if (!rc) return 0;
			std::cout << "Incorrect input, please, try again:";
		} while (rc < 0);

		try {
			elem.disconnect_conn(a - 1);
		}
		catch (std::runtime_error &rt) {
			std::cout << rt.what() << std::endl;
		}
		return 1;
	}

	int d_connect_conn(element& elem) {
		int rc, a, b;

		std::cout << "Input connection\'s current id to connect:" << std::endl;
		do {
			rc = input_number(a, std::cin);
			if (rc == 1) break;
			if (!rc) return 0;
			std::cout << "Incorrect input, please, try again:";
		} while (rc < 0);

		std::cout << "Input target connection id:" << std::endl;
		do {
			rc = input_number(b, std::cin);
			if (rc == 1) break;
			if (!rc) return 0;
			std::cout << "Incorrect input, please, try again:";
		} while (rc < 0);

		try {
			elem.connect_conns(a - 1, b - 1);
		}
		catch (std::runtime_error &rt) {
			std::cout << rt.what() << std::endl;
		}
		return 1;
	}

	int d_change_all_states(element& elem) {
		reorg(std::cin,elem);
		return 1;
	}

	int d_print_conn_state(element& elem) {
		int rc, b;
		const element elemc = elem;

		std::cout << "Input connection\'s current id to get its\' state:" << std::endl;
		do {
			rc = input_number(b, std::cin);
			if (rc == 1) break;
			if (!rc) return 0;
			std::cout << "Incorrect input, please, try again:";
		} while (rc < 0);

		try {
			std::cout << elemc.get_state(b - 1);
		}
		catch (std::runtime_error &rt) {
			std::cout << rt.what() << std::endl;
		}
		return 1;
	}

	int d_set_conn_state(element& elem) {
		int rc, b, a;

		std::cout << "Input connection\'s current id to set state:" << std::endl;
		do {
			rc = input_number(b, std::cin);
			if (rc == 1) break;
			if (!rc) return 0;
			std::cout << "Incorrect input, please, try again:";
		} while (rc < 0);

		std::cout << "Input new state(0 - low, 1 - high, else - X)" << std::endl;
		do {
			rc = input_number(a, std::cin);
			if (rc == 1) break;
			if (!rc) return 0;
			std::cout << "Incorrect input, please, try again:";
		} while (rc < 0);

		try {
			elem.set_state(b - 1) = static_cast<conditions>(a);
		}
		catch (std::runtime_error &rt) {
			std::cout << rt.what() << std::endl;
		}
		
		return 1;
	}

	connection &connection::set_cond(int new_state) {
		if (type == IM)
			throw std::runtime_error("there is no such connection");
		bool lonely = true;
		for (int i = 0; i < 3; ++i) {
			if (sockets[i] != -1) {
				lonely = false;
				break;
			}
		}
		if (lonely)
			throw std::runtime_error("only X state permitted for lonely connections");

		switch (new_state) {
		case 0:
			condition = LOW;
			break;
		case 1:
			condition = HIGH;
			break;
		default:
			condition = X;
			break;
		}
		return *this;
	}
}

