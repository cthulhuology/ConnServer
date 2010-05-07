// Cache Class
//
// © 2010 David J. Goehrig
// 

Cache* Cache::cache = NULL;

Cache::Cache() {};
Cache::~Cache() {};

bool 
Cache::create(const string& s)
{
	if (cache != NULL) return false;

	cache = new Cache();
	
	string load_cache = "SELECT value FROM objects WHERE id = " + s;
	Database* d = DBPool::grab();
	d->begin();
	Result* res = d->query(load_cache);

	if (res == NULL || ! res->success() || res->rows != 1) {
		cerr << "[Cache::init] failed " << d->error() << endl;
		cache->next_guid = Uint64_of_string(s);
		cache->idspace = Uint64_of_string(s);
		cache->id = Object::create(d,cache->type());
		cache->save(d);
		d->commit();
		delete res;
		DBPool::release(d);
		return true;
	} else {
		string c = (*res)["value"][0];
		d->commit();
		delete res;
		DBPool::release(d);
		cache->id = Uint64_of_string(s);
		return cache->init(c);
	}
}

bool
Cache::init(const string& s)
{
	map<string,string> o = parse(s);
	idspace = Uint64_of_string(o["idspace"]);
	next_guid = Uint64_of_string(o["next_guid"]);
	map<string,string>::iterator i;
	for (i = o.begin(); i != o.end(); ++i) {
		if (i->first != "idspace" && i->first != "next_guid" && i->first != "type") {
			allocated[i->first] = int_of_string(i->second);
		}
	}
	return true;
END

TOS_METHOD(Cache)
	string stats;
	map<string,int>::iterator i;
	for (i = allocated.begin(); i != allocated.end(); ++i) {
		if (stats.empty()) {
			stats = "," + i->first + ":" + string_of_int(i->second);
		} else {
			stats += "," + i->first + ":" + string_of_int(i->second);
		}
	}
	return "type:" + type()  
		+ ",idspace:" + string_of_Uint64(idspace)
		+ ",next_guid:" + string_of_Uint64(next_guid)
		+ stats;
END

bool
Cache::fini()
{
	if (cache == NULL) return false;
	cerr << "[Cache::fini] " << cache->tos() << endl;
	Database* d = DBPool::grab();
	d->begin();
	if (!cache->save(d) || ! d->commit()) {
		d->rollback();
		return false;
	}
	map<UInt64,ID>::iterator i;
	for (i = cache->obj_id_map.begin(); i != cache->obj_id_map.end(); ++i) {
		delete i->second;
	}
	DBPool::release(d);
	delete cache;
	cache = NULL;
	return true;
}

bool
Cache::store()
{
	if (cache == NULL) return false;
	bool retval = true;
	map<UInt64,ID>::iterator i;
	Database* d = DBPool::grab();
	d->begin();
	for (i = cache->obj_id_map.begin(); i != cache->obj_id_map.end(); ++i) {
		if (i->second) {
			retval &= i->second->save(d);
		}
	}
	retval &= cache->save(d);
	if (retval) {
		d->commit();
	} else {
		d->rollback();
	}
	DBPool::release(d);
	return retval;
}

UInt64
Cache::next()
{
	Database* d = DBPool::grab();
	d->begin();
	UInt64 retval = cache->next_guid;
	cache->next_guid += 1;
	if (! cache->save(d) || ! d->commit()) {
		cerr << "[Cache::next] failed to allocate guid" << endl;
		d->rollback();
		DBPool::release(d);
		return Cache::next();
	}
	DBPool::release(d);
	cerr << "[Cache::next] allocating " << retval << endl;
	return retval;
}

bool
Cache::add(ID obj, bool alloced)
{
	if (obj == NULL || obj->id == 0LL) return false;
	cache->obj_id_map[obj->id] = obj;
	if (alloced) cache->allocated[obj->type()] += 1;
	return true;
}

bool
Cache::update(Message&  m)
{
	if (m["id"].empty() || m["obj"].empty()) return false;
	UInt64 objid = Uint64_of_string(m["id"]);
	ID obj = cache->obj_id_map[objid];
	if (obj == NULL) return true; // Object not in cache
	obj->init(m["obj"]);
}

string
Cache::statistics()
{
	string stats = "msg=cache-stats";
	map<string,int>::iterator i,j;
	for (i = cache->allocated.begin(); i != cache->allocated.end(); ++i) {
		stats += "&" + i->first + ".allocated=" 
			+ string_of_int(i->second);
	}
	for (j = cache->cached.begin(); j != cache->cached.end(); ++j) {
		stats += "&" + j->first + ".cached=" 
			+ string_of_int(j->second);
	}
	return stats;
}
