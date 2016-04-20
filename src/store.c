#include "store.h"
#include "dbkey.h"

int SGDB_init(){
	u_int32_t flags;
	u_int32_t env_flags;SGDB_Env
	int ret;
	
	ret = db_env_create(&SGDB_Env, 0);
	if (ret != 0) {
		fprintf(stderr, "Error creating env handle: %s\n", db_strerror(ret));
		return -1;
	}
	
	/* Open the environment. */
	env_flags = DB_CREATE |    /* If the environment does not exist, create it. */
				DB_INIT_MPOOL; /* Initialize the in-memory cache. */
	ret = SGDB_Env->open(SGDB_Env,
					  SGDB_ENV_DIR,
					  env_flags,
					  0);
	if (ret != 0) {
		fprintf(stderr, "Environment open failed: %s", db_strerror(ret));
		return -1;
	}

	ret = db_create(&SGDB_dbp, SGDB_Env, 0);
	if (ret != 0) {
		fprintf(stderr, "Error creating db handler: %s", db_strerror(ret));
		return -1;
	}
	
	SGDB_dbp->set_bt_compare(SGDB_dbp, compare_dbkey); //set the comparator, must before open

	flags = DB_CREATE; //DB_EXCL, DB_RDONLY, DB_TRUNCATE; DB->get_open_flags()
	ret = SGDB_dbp->open(SGDB_dbp,
					NULL,
					SGDB_DB_FILE,
					NULL,
					DB_BTREE,
					flags,
					0);
	if (ret != 0) {
		fprintf(stderr, "Database open failed: %s", db_strerror(ret));
		return -1;
	}
}

void SGDB_close(){
	if (SGDB_dbp != NULL)
		SGDB_dbp->close(SGDB_dbp, 0);
	if (SGDB_Env != NULL)
		SGDB_Env->close(SGDB_Env, 0);
	//You can further remove the database
	//SGDB_dbp->remove(SGDB_dbp, dbf, NULL, 0);
}

int SGDB_insert(char *src, u_int32_t ssize, char *dst, u_int32_t dsize, u_int32_t type, u_int64_t ts, char *val, u_int32_t vsize){
	DBKey dbkey;
	Slice _a = {ssize, src};
	Slice _b = {dsize, dst};
	dbkey.src = _a;
	dbkey.dst = _b;
	dbkey.ts = ts;
	dbkey.type = type;
	int ret;
	
	DBT key, value;
	/* Zero out the DBTs before using them. */
	memset(&key, 0, sizeof(DBT));
	memset(&value, 0, sizeof(DBT));
	
	key.data = decompose(dbkey, &key.size);
	
	value.size = vsize;
	value.data = val;
	
	ret = SGDB_dbp->put(SGDB_dbp, NULL, &key, &value, DB_NOOVERWRITE);
	if (ret == DB_KEYEXIST) {
	    SGDB_dbp->err(SGDB_dbp, ret,
	      "Put failed because key %f already exists", money);
	}
	return ret;
}

char* SGDB_get(char *src, u_int32_t ssize, char *dst, u_int32_t dsize, u_int32_t type, u_int64_t ts, u_int32_t *vsize){
	DBKey dbkey;
	Slice _a = {ssize, src};
	Slice _b = {dsize, dst};
	dbkey.src = _a;
	dbkey.dst = _b;
	dbkey.ts = ts;
	dbkey.type = type;
	
	DBT key, value;
	/* Zero out the DBTs before using them. */
	memset(&key, 0, sizeof(DBT));
	memset(&value, 0, sizeof(DBT));
	
	key.data = decompose(dbkey, &key.size);
	
	int ret = SGDB_dbp->get(SGDB_dbp, NULL, &key, &value, 0);
	*vsize = value.size;
	return value.data;
}

int SGDB_del(char *src, u_int32_t ssize, char *dst, u_int32_t dsize, u_int32_t type, u_int64_t ts){
	DBKey dbkey;
	Slice _a = {ssize, src};
	Slice _b = {dsize, dst};
	dbkey.src = _a;
	dbkey.dst = _b;
	dbkey.ts = ts;
	dbkey.type = type;
	int ret;
	
	DBT key, value;
	/* Zero out the DBTs before using them. */
	memset(&key, 0, sizeof(DBT));
	memset(&value, 0, sizeof(DBT));
	
	key.data = decompose(dbkey, &key.size);
	ret = SGDB_dbp->del(SGDB_dbp, NULL, &key, 0);
	return ret;
}

void __insert_example(){
    
	MY_STRUCT user;
	/* Initialize the structure */
	memset(&user, 0, sizeof(MY_STRUCT));
	user.id = 1;
	key.data = &user.id;
	key.size = sizeof(int);
	/* Use our memory to retrieve the structure */
	data.data = &user;
	data.ulen = sizeof(MY_STRUCT);
	data.flags = DB_DBT_USERMEM;
	SGDB_dbp->get(SGDB_dbp, NULL, &key, &data, 0);
	
	
	SGDB_dbp->del(SGDB_dbp, NULL, &key, 0);
}
