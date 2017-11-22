/******************************************************************************
*   TinTin++                                                                  *
*   Copyright (C) 2005 (See CREDITS file)                                     *
*                                                                             *
*   This program is protected under the GNU GPL (See COPYING)                 *
*                                                                             *
*   This program is free software; you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 2 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with this program; if not, write to the Free Software               *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
*******************************************************************************/

/******************************************************************************
*              (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                   *
*                                                                             *
*                     coded by Igor van den Hoven 2004                        *
******************************************************************************/


#include "tintin.h"


DO_COMMAND(do_class)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE];
	int i;

	struct listroot *root;
	struct listnode *node;


	root = ses->list[LIST_CLASS];

	arg = sub_arg_in_braces(ses, arg, arg1, 0, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, 0, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, 0, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		tintin_header(ses, " CLASSES ");

		for (root->update = 0 ; root->update < root->used ; root->update++)
		{
			node = root->list[root->update];

			tintin_printf2(ses, "%-20s %3d %s", node->left, count_class(ses, node), !strcmp(ses->group, node->left) ? "OPEN" : "CLOSED");
		}
	}
	else if (*arg2 == 0)
	{
		class_list(ses, arg1, arg2);
	}
	else
	{
		for (i = 0 ; *class_table[i].name ; i++)
		{
			if (is_abbrev(arg2, class_table[i].name))
			{
				break;
			}
		}

		if (*class_table[i].name == 0)
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: CLASS {name} {OPEN|CLOSE|READ|WRITE|KILL}.", arg1, capitalize(arg2));
		}
		else
		{
			if (!search_node_list(ses->list[LIST_CLASS], arg1))
			{
				update_node_list(ses->list[LIST_CLASS], arg1, arg2, arg3);
			}
			class_table[i].group(ses, arg1, arg3);
		}
	}
	return ses;
}


int count_class(struct session *ses, struct listnode *group)
{
	int list, cnt, index;

	for (cnt = list = 0 ; list < LIST_MAX ; list++)
	{
		if (!HAS_BIT(ses->list[list]->flags, LIST_FLAG_CLASS))
		{
			continue;
		}

		for (index = 0 ; index < ses->list[list]->used ; index++)
		{
			if (!strcmp(ses->list[list]->list[index]->group, group->left))
			{
				cnt++;
			}
		}
	}
	return cnt;
}


DO_CLASS(class_open)
{
	if (!search_node_list(ses->list[LIST_CLASS], left))
	{
		update_node_list(ses->list[LIST_CLASS], left, "", "");
	}

	if (!strcmp(ses->group, left))
	{
		show_message(ses, LIST_CLASS, "#CLASS {%s} IS ALREADY OPENED.", left);

	}
	else
	{
		RESTRING(ses->group, left);

		show_message(ses, LIST_CLASS, "#CLASS {%s} HAS BEEN OPENED.", left);
	}
	return ses;
}

DO_CLASS(class_close)
{
	if (!search_node_list(ses->list[LIST_CLASS], left))
	{
		show_message(ses, LIST_CLASS, "#CLASS {%s} DOES NOT EXIST.", left);
	}
	else
	{
		if (!strcmp(ses->group, left))
		{
			RESTRING(ses->group, "");

			show_message(ses, LIST_CLASS, "#CLASS {%s} HAS BEEN CLOSED.", left);
		}
		else
		{
			show_message(ses, LIST_CLASS, "#CLASS {%s} IS ALREADY CLOSED.", left);
		}
	}
	return ses;
}

DO_CLASS(class_list)
{
	int i, j;

	if (search_node_list(ses->list[LIST_CLASS], left))
	{
		tintin_header(ses, " %s ", left);

		for (i = 0 ; i < LIST_MAX ; i++)
		{
			if (*right && !is_abbrev(right, list_table[i].name_multi))
			{
				continue;
			}

			for (j = 0 ; j < ses->list[i]->used ; j++)
			{
				if (!strcmp(ses->list[i]->list[j]->group, left))
				{
					show_node(ses->list[i], ses->list[i]->list[j], 0);
				}
			}
		}
	}
	else
	{
		show_error(ses, LIST_CLASS, "#CLASS {%s} DOES NOT EXIST.", left);
	}
	return ses;
}

DO_CLASS(class_read)
{
	if (!search_node_list(ses->list[LIST_CLASS], left))
	{
		update_node_list(ses->list[LIST_CLASS], left, "", "");
	}
	class_open(ses, left, right);

	do_read(ses, right);

	class_close(ses, left, right);

	return ses;
}


DO_CLASS(class_write)
{
	FILE *file;
	int list, index;

	if (!search_node_list(ses->list[LIST_CLASS], left))
	{
		return show_error(ses, LIST_CLASS, "#CLASS {%s} DOES NOT EXIST.", left);
	}

	if (*right == 0 || (file = fopen(right, "w")) == NULL)
	{
		return show_error(ses, LIST_CLASS, "#ERROR: #CLASS WRITE {%s} - COULDN'T OPEN FILE TO WRITE.", right);
	}

	fprintf(file, "%cCLASS {%s} OPEN\n\n", gtd->tintin_char, left);

	for (list = 0 ; list < LIST_MAX ; list++)
	{
		if (!HAS_BIT(ses->list[list]->flags, LIST_FLAG_CLASS))
		{
			continue;
		}

		for (index = 0 ; index < ses->list[list]->used ; index++)
		{
			if (!strcmp(ses->list[list]->list[index]->group, left))
			{
				write_node(ses, list, ses->list[list]->list[index], file);
			}
		}
	}

	fprintf(file, "\n%cCLASS {%s} CLOSE\n", gtd->tintin_char, left);

	fclose(file);

	show_message(ses, LIST_CLASS, "#CLASS {%s} HAS BEEN WRITTEN TO FILE.", left);

	return ses;
}

DO_CLASS(class_kill)
{
	int type, index, group;

	if (!search_node_list(ses->list[LIST_CLASS], left))
	{
		update_node_list(ses->list[LIST_CLASS], left, "", "");
	}
	group = search_index_list(ses->list[LIST_CLASS], left, NULL);

	for (type = 0 ; type < LIST_MAX ; type++)
	{
		if (!HAS_BIT(ses->list[type]->flags, LIST_FLAG_CLASS))
		{
			continue;
		}

		for (index = 0 ; index < ses->list[type]->used ; index++)
		{
			if (!strcmp(ses->list[type]->list[index]->group, left))
			{
				delete_index_list(ses->list[type], index--);
			}
		}
	}

	delete_index_list(ses->list[LIST_CLASS], group);

	if (!strcmp(ses->group, left))
	{
		RESTRING(ses->group, "");
	}

	show_message(ses, LIST_CLASS, "#CLASS {%s} HAS BEEN KILLED.", left);

	return ses;
}
