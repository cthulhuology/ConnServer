#!/bin/bash
echo "
-- schema.sql
-- 
-- objects
--
-- A table containing all of the objects in the system
--

drop table if exists objects;
create table objects (
	id bigint primary key,
	type text default '',
	value text default '',
	created timestamp,
	updated timestamp
);

-- a function to create new objects
create or replace function new_object(_id bigint, _type text, _value text) returns bigint as \$\$
begin
	insert into objects (id,type,value,created,updated) values (_id,_type,_value, timestamp 'now', timestamp 'now');
	return _id;
end;
\$\$ language plpgsql;

-- a function to load an object by id
create or replace function load_object(_id bigint) returns text as \$\$
declare
	_value text;
begin
	select into _value value from objects where id = _id;
	if not found then raise exception 'object % not found', _id; end if;
	return _value;	
end;
\$\$ language plpgsql;

-- a function to update an object by id 
create or replace function save_object(_id bigint, _value text) returns boolean as \$\$
declare
	_check bigint;
begin
	select into _check id from objects where id = _id;
	if not found then return false; end if;
	update objects set value = _value, updated = timestamp 'now'  where id = _id;
	return true;
end;
\$\$ language plpgsql;

-- a function to get all objects of a type
create or replace function find_objects(_type text) returns setof bigint as \$\$
begin
	return query select id from objects where type = _type;
end;
\$\$ language plpgsql;
"
