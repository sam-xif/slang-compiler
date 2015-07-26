
#include "includes.h"


FILE *fp;
FILE *lang_txt;

// This is platform dependent. Maybe add some preprocessor directives here and there


char *ExePath() {
#ifdef _WINDOWS

	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);

	int pos;
	for (int i = strlen(buffer) - 1; i >= 0; i--) {
		if (buffer[i] == '\\' || buffer[i] == '/') {
			pos = i;
			break;
		}
	}
	char subString[MAX_PATH];
	strncpy_s(subString, MAX_PATH, buffer, pos + 1);
	return subString;


#else

	// TODO: Add stuff for Linux-type systems

#endif
}

// Entry point
int main(int argc, char **argv) {
	
	RESET_ERRNO;
	global_slang_errno_state = slang_create_errno_state();
	
	fp = NULL;
	lang_txt = NULL;
	scanner_t *scan;

	int file_length = 0;
	char *buf = NULL;

	const char *path = "C:\\Users\\Samuel\\Documents\\visual studio 2013\\Projects\\Compiler2\\Debug\\lang.txt";
	const char *path2 = "C:\\Users\\Samuel\\Documents\\visual studio 2013\\Projects\\Compiler2\\Debug\\sample_code.txt";
	errno_t err = fopen_s(&fp, path, "rb+");
	if (!err) {
		fseek(fp, 0, SEEK_END);
		file_length = (int)ftell(fp);
		fseek(fp, 0, SEEK_SET);
		buf = (char *)malloc((sizeof(char) * file_length) + 1);
		fread(buf, sizeof(char), file_length, fp);
		buf[file_length] = '\0'; // Add null-terminating character to the end of the buffer

		if (fclose(fp) == EOF) {
			errno = SLANG_IO_ERR;
			raise_error("main.c", slang_create_error_token(59, 0), NULL);
			return;
		}

		// Sample code string
		//char *sample_code = "if hello == hello then \n{\nxyz = (asd == 22)\n} else {\na = b\n}";
		//char *sample_code = "func example_func(int x, int y) returns void\n{\nif asdf == (2*5 == -4) then \n{\nxyz = (asd == 22)\n} else {\na = b\n}\n\nint x = { f\n }\n}\n\n\n";
		char *sample_code = NULL;
		int sample_code_length = 0;
		errno_t err = fopen_s(&fp, path2, "rb+");
		if (err) {
			return;
		}
		fseek(fp, 0, SEEK_END);
		sample_code_length = (int)ftell(fp);
		fseek(fp, 0, SEEK_SET);
		sample_code = (char *)malloc((sizeof(char) * sample_code_length) + 1);
		fread(sample_code, sizeof(char), sample_code_length, fp);
		sample_code[sample_code_length] = '\0'; // Add null-terminating character to the end of the buffer
		//free(sample_code);
		//sample_code = "func example_func(int x, int y) returns void\n{\nif (asdf == (2*5 == -4)) then \n{\nxyz = (asd == 22)\n} else {\na = b\n}\n\nint x = { f\n }\n}\n\n\n";
		if (fclose(fp) == EOF) {
			errno = SLANG_IO_ERR;
			raise_error("main.c", slang_create_error_token(59, 0), NULL);
			return;
		}

		int i;

		lang_state_t *state = lang_txt_parse_buffer(buf, file_length);
		cfg_t *cfg = lang_get_cfg(state);

		scanner_t *scan = scanner_alloc();
		scan_buffer(sample_code, strlen(sample_code), scan);

		// Remove all tabs and spaces in the parsed token list.
		remove_all_tokens_by_type(scan->token_list, token_tab);
		remove_all_tokens_by_type(scan->token_list, token_space);

		// Remove all carriage return symbols that are created by text editors on windows.
		remove_all_tokens_by_type(scan->token_list, token_cr);

		parser_t *parse = parser_alloc(cfg);
		pt_node_t *p = parse_recursive_descent(scan->token_list, parse);

		CLEAR_ERROR_QUEUE;

		if (p != NULL) { 

			node_list_t *leaves = pt_get_leaves(p);

			//list_free(leaves, NULL);
			// Reorganizes the nodes to be more coherent and sensical.
			parse_rd_rewrite_tree(p);

			leaves = pt_get_leaves(p);

			// To get all the functions defined within the parsed code:
			node_list_t *func_nodes = pt_get_nodes_by_name(p, "EXPR");

			parser_free_pt(p, true);
			parse->result = NULL;
		}

		// Clean up

		parser_free(parse, false);
		scan->token_list = NULL;
		scanner_free(scan, false);
		cfg_free(cfg);
		lang_free(state);

		/* 
		 * Be careful when freeing a buffer, because all tokens scanned from the buffer point to it.
		 * Therefore, if you release the buffer, all the tokens will lose their data. Free the buffer
		 * only when you are sure the tokens are finished being used (or at the end of program execution).
		 */
		free(buf);

		slang_free_errno_state(global_slang_errno_state);
	}
	else {
		slang_error_t error;
		MALLOC_STRING("main.c", error.file_name);
		error.error_code = errno;
		slang_queue_error(&error, global_slang_errno_state);
		return 1;
	}

	CLEAR_ERROR_QUEUE;
	getchar();

	return 0;
}