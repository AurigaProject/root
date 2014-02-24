#include <stdio.h>
#include <stdlib.h>

#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/utils.h"
#include "converter.h"


struct accreg {
	int account_id;
	int reg_num;
	struct global_reg reg[ACCOUNT_REG_NUM];
};

struct status_change_data {
	int account_id;
	int char_id;
	short count;
	struct {
		short type;
		int val1,val2,val3,val4,tick;
	} data[MAX_STATUSCHANGE];
};

// アカウント変数を文字列から変換
static int int_accreg_fromstr(const char *str,struct accreg *reg)
{
	int j,n,val;
	char buf[128];
	const char *p = str;

	if(sscanf(p,"%d\t%n", &reg->account_id, &n) != 1 || reg->account_id <= 0)
		return 1;

	for(j=0, p+=n; j<ACCOUNT_REG_NUM; j++, p+=n) {
		if(sscanf(p, "%[^,],%d%n", buf, &val, &n) != 2)
			break;
		strncpy(reg->reg[j].str, buf, 32);
		reg->reg[j].value = val;
		n++;
	}
	reg->reg_num = j;

	return 0;
}

// アカウント変数を書き込み
static void int_accreg_tosql(struct accreg *reg) {
	int j;
	char buf[128];

	//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
	sprintf(tmp_sql, "DELETE FROM `global_reg_value` WHERE `type`=2 AND `account_id`='%d'", reg->account_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (delete `global_reg_value`)- %s\n", mysql_error(&mysql_handle) );
	}

	for(j=0; j<reg->reg_num; j++) {
		if(reg->reg[j].str[0]) {
			sprintf(tmp_sql, "INSERT INTO `global_reg_value` (`type`, `account_id`, `str`, `value`) VALUES (2,'%d', '%s','%d')",
				reg->account_id, strecpy(buf,reg->reg[j].str), reg->reg[j].value);
			if(mysql_query(&mysql_handle, tmp_sql) ) {
				printf("DB server Error (insert `global_reg_value`)- %s\n", mysql_error(&mysql_handle) );
			}
		}
	}

	return;
}

// ステータス異常データを文字列から変換
static int int_status_fromstr(char *str, struct status_change_data *sc)
{
	int i,next,set,len;
	int tmp_int[6];

	if(sscanf(str,"%d,%d%n",&sc->char_id,&sc->account_id,&next) != 2)
		return 1;

	if(sc->account_id <= 0 || sc->char_id <= 0)
		return 1;

	if(str[next] == '\n' || str[next] == '\r')
		return 1;	// account_idとchar_idだけの行は有り得ない
	next++;

	for(i=0; str[next] && str[next] != '\t'; i++) {
		set = sscanf(str+next,"%d,%d,%d,%d,%d,%d%n",
			&tmp_int[0],&tmp_int[1],&tmp_int[2],&tmp_int[3],&tmp_int[4],&tmp_int[5],&len);
		if(set != 6)
			return 1;
		if(i < MAX_STATUSCHANGE && tmp_int[0] >= 0 && tmp_int[0] < MAX_STATUSCHANGE) {
			sc->data[i].type = (short)tmp_int[0];
			sc->data[i].val1 = tmp_int[1];
			sc->data[i].val2 = tmp_int[2];
			sc->data[i].val3 = tmp_int[3];
			sc->data[i].val4 = tmp_int[4];
			sc->data[i].tick = tmp_int[5];
		}
		next+=len;
		if(str[next] == ' ')
			next++;
	}
	sc->count = (i < MAX_STATUSCHANGE)? i: MAX_STATUSCHANGE;

	return 0;
}

// ステータス異常データを書き込み
static void int_status_tosql(struct status_change_data *sc)
{
	int i;

	sprintf(tmp_sql,"DELETE FROM `status_change` WHERE `char_id`='%d'",sc->char_id);
	if(mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (delete `status_change`)- %s\n", mysql_error(&mysql_handle));
	}

	for(i=0; i<sc->count; i++) {
		sprintf(
			tmp_sql,
			"INSERT INTO `status_change` (`char_id`, `account_id`, `type`, `val1`, `val2`, `val3`, `val4`, `tick`) "
			"VALUES ('%d','%d','%d','%d','%d','%d','%d','%d')",
			sc->char_id, sc->account_id, sc->data[i].type,
			sc->data[i].val1, sc->data[i].val2, sc->data[i].val3, sc->data[i].val4, sc->data[i].tick
		);
		if(mysql_query(&mysql_handle, tmp_sql)) {
			printf("DB server Error (insert `status_change`)- %s\n", mysql_error(&mysql_handle));
		}
	}

	return;
}

// メールデータを文字列から変換
static int int_mail_fromstr(char *str,struct mail *m)
{
	int tmp_int[4];

	if( sscanf(str,"%d,%d,%u,%d",&tmp_int[0],&tmp_int[1],&tmp_int[2],&tmp_int[3]) != 4 )
		return 1;

	m->char_id= tmp_int[0];
	m->account_id = tmp_int[1];
	m->rates = (unsigned int)tmp_int[2];
	m->store = tmp_int[3];

	return 0;
}

// メールデータの削除
static int int_mail_delete_fromsql(int char_id)
{
	sprintf(tmp_sql, "DELETE FROM `mail` WHERE `char_id` = '%d'", char_id);
	if(mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (delete `mail`)- %s\n", mysql_error(&mysql_handle));
	}
	return 0;
}

// メールデータを書き込み
static int int_mail_tosql(struct mail* m)
{
	int_mail_delete_fromsql(m->char_id);

	// `mail` (`char_id`, `account_id`, `rates`, `store`)
	sprintf(
		tmp_sql, "INSERT INTO `mail` (`char_id`, `account_id`, `rates`, `store`) VALUES ('%d','%d','%u','%d')",
		m->char_id, m->account_id, m->rates, m->store
	);
	if(mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (insert `mail`)- %s\n", mysql_error(&mysql_handle));
	}

	return 0;
}

// 個人メールBOXを文字列から変換
static int int_mailbox_fromstr(char *str,struct mail_data *md,char *body_data)
{
	int n;
	int tmp_int[16];
	char tmp_char[3][1024];

	n=sscanf(str,"%u,%d\t%[^\t]\t%[^\t]\t%[^\t]\t%d\t%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\t%d\t%d\t%[^\r\n]",
		&tmp_int[0],&tmp_int[1],tmp_char[0],tmp_char[1],tmp_char[2],&tmp_int[2],
		&tmp_int[3],&tmp_int[4],&tmp_int[5],&tmp_int[6],&tmp_int[7],&tmp_int[8],&tmp_int[9],&tmp_int[10],&tmp_int[11],&tmp_int[12],&tmp_int[13],
		&tmp_int[14],&tmp_int[15],body_data);
	if(n!=20)
		return 1;

	md->mail_num = (unsigned int)tmp_int[0];
	md->read     = tmp_int[1];
	memcpy(md->char_name,tmp_char[0],24);
	memcpy(md->receive_name,tmp_char[1],24);
	memcpy(md->title,tmp_char[2],40);
	md->zeny           = tmp_int[2];
	md->item.id        = tmp_int[3];
	md->item.nameid    = tmp_int[4];
	md->item.amount    = tmp_int[5];
	md->item.equip     = tmp_int[6];
	md->item.identify  = tmp_int[7];
	md->item.refine    = tmp_int[8];
	md->item.attribute = tmp_int[9];
	md->item.card[0]   = tmp_int[10];
	md->item.card[1]   = tmp_int[11];
	md->item.card[2]   = tmp_int[12];
	md->item.card[3]   = tmp_int[13];
	md->times          = (unsigned int)tmp_int[14];
	md->body_size      = (unsigned int)tmp_int[15];
	// md->bodyは使わない

	return 0;
}

// 個人メールBOXを書き込み
static int int_mailbox_tosql(struct mail_data *md,char *body_data)
{
	char buf[3][256];

	sprintf(tmp_sql, "DELETE FROM `mail_data` WHERE `char_id` = '%d' AND `number` = '%d'", md->char_id, md->mail_num);
	if(mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (delete `mail_data`)- %s\n", mysql_error(&mysql_handle));
	}

	sprintf(
		tmp_sql,
		"INSERT INTO `mail_data` (`char_id`, `number`, `read`, `send_name`, `receive_name`, `title`, "
		"`times`, `size`, `body`, `zeny`, "
		"`id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, "
		"`card0`, `card1`, `card2`, `card3`) "
		"VALUES ('%d','%u','%d','%s','%s','%s','%u','%u','%s','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d')",
		md->char_id, md->mail_num, md->read, strecpy(buf[0],md->char_name), strecpy(buf[1],md->receive_name), strecpy(buf[2],md->title),
		md->times, md->body_size, body_data, md->zeny,
		md->item.id, md->item.nameid, md->item.amount, md->item.equip, md->item.identify, md->item.refine, md->item.attribute,
		md->item.card[0], md->item.card[1], md->item.card[2], md->item.card[3]
	);
	if(mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (insert `mail_data`)- %s\n", mysql_error(&mysql_handle));
	}

	return 0;
}

int inter_convert(void)
{
	char input, line[65536];
	int c = 0;
	FILE *fp;

	printf("\nDo you wish to convert your Account Registered Variables to SQL? (y/n) : ");
	input = getchar();
	if(input == 'y' || input == 'Y') {
		struct accreg reg;
		printf("\nConverting Account Registered Variables...\n");
		if( (fp = fopen(account_reg_txt,"r")) == NULL )
			return 0;
		while(fgets(line, sizeof(line), fp)) {
			c++;
			memset(&reg, 0, sizeof(reg));
			if(int_accreg_fromstr(line, &reg) == 0 && reg.account_id > 0) {
				int_accreg_tosql(&reg);
			} else {
				printf("int_accreg: broken data [%s] line %d\n", account_reg_txt, c);
			}
		}
		fclose(fp);
	}
	while(getchar() != '\n');

	c = 0;
	printf("\nDo you wish to convert your Status Change Data to SQL? (y/n) : ");
	input = getchar();
	if(input == 'y' || input == 'Y') {
		struct status_change_data sc;
		printf("\nConverting Status Change Data...\n");
		if( (fp = fopen(scdata_txt,"r")) == NULL )
			return 0;
		while(fgets(line, sizeof(line), fp)) {
			c++;
			memset(&sc, 0, sizeof(sc));
			if(int_status_fromstr(line, &sc) == 0) {
				int_status_tosql(&sc);
			} else {
				printf("int_status: broken data [%s] line %d\n", scdata_txt, c);
			}
		}
		fclose(fp);
	}
	while(getchar() != '\n');

	c = 0;
	printf("\nDo you wish to convert your Mail Data to SQL? (y/n) : ");
	input = getchar();
	if(input == 'y' || input == 'Y') {
		struct mail m;
		struct mail_data md;
		int list_num=0, list_size, i;
		int *charid_list;

		printf("\nConverting Mail Data...\n");

		// mail.txt
		if( (fp = fopen(mail_txt,"r")) == NULL )
			return 0;
		list_size = 256;
		charid_list = (int *)aCalloc(list_size, sizeof(int));
		while(fgets(line, sizeof(line), fp)) {
			c++;
			memset(&m, 0, sizeof(m));
			if(int_mail_fromstr(line, &m) == 0) {
				int_mail_tosql(&m);
				if(list_num >= list_size) {
					list_size += 256;
					charid_list = (int *)aRealloc(charid_list, list_size*sizeof(int));
					memset(charid_list+(list_size-256), 0, 256*sizeof(int));
				}
				charid_list[list_num++] = m.char_id;	// キャラIDをリストに保存
			} else {
				printf("int_mail: broken data [%s] line %d\n", mail_txt, c);
			}
		}
		fclose(fp);

		// mail_data
		for(i=0; i<list_num; i++) {
			char filename[1024];
			sprintf(filename,"%s%d.txt",mail_dir,charid_list[i]);
			if( (fp = fopen(filename,"r")) == NULL ) {
				printf("int_mail: [%s] not found!!\n", filename);
				int_mail_delete_fromsql(charid_list[i]);	// 整合性が取れないので `mail` から削除
				continue;
			}
			c = 0;
			while(fgets(line, sizeof(line), fp)) {
				char body_data[1024];	// bodyはバイナリデータのままSQLに流し込むのでUNHEXしない
				c++;
				memset(&md, 0, sizeof(md));
				if(int_mailbox_fromstr(line, &md, body_data) == 0) {
					md.char_id = charid_list[i];
					int_mailbox_tosql(&md, body_data);
				} else {
					printf("int_mail: broken data [%s] line %d\n", filename, c);
				}
			}
			fclose(fp);
		}
		aFree(charid_list);
	}
	while(getchar() != '\n');

	return 0;
}
