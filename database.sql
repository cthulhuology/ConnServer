-- schema.sql
-- 
-- cache_table
--
-- 	This table caches all of the poker data
--	We are largely database agnostic
--

drop table if exists cache_table;
create table cache_table (
	id bigint not NULL,
	value text default ''
);

