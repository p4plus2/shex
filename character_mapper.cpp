#include "character_mapper.h"
#include <QFile>

bool character_mapper::load_map(QString map_file_path)
{
	if(map == nullptr){
		map = new QMap<unsigned char, unsigned char>();
	}else{
		map->clear();
	}
	QFile map_file(map_file_path);
	map_file.open(QIODevice::ReadOnly | QIODevice::Text);
	while(!map_file.atEnd()){
		QByteArray line = map_file.readLine();
		int seperator = line.lastIndexOf('=');
		if(seperator == -1){
			return (map_state = false);
		}
		QString left = line.left(seperator).trimmed();
		unsigned char key;
		if(left.isEmpty()){
			key = ' ';
		}else if(left == "\\t"){
			key = '\t';
		}else if(left == "\\r"){
			key = '\r';
		}else if(left == "\\n"){
			key = '\n';
		}else{
			key = left.at(0).unicode();
		}
		QString right = line.right(line.length()-seperator-1).trimmed();
		bool status;
		unsigned char value = right.toInt(&status, 16);
		if(right.length() != 2 || !status){
			return (map_state = false);
		}
		map->insert(key, value);
		
	}
	return (map_state = true);
}

void character_mapper::set_map(QMap<unsigned char, unsigned char> *dialog_mapper)
{
	if(map != nullptr && map != dialog_mapper){
		delete map;
	}
	map = dialog_mapper;
}

unsigned char character_mapper::decode(unsigned char input)
{
	if(map != nullptr && map->contains(input) && map_state){
		return map->value(input);
	}
	return input;
	
}

QByteArray character_mapper::decode(QByteArray input)
{
	if(map != nullptr && map_state){
		for(int i = input.length() - 1; i > -1; i--){
			unsigned char current = input.at(i);
			input[i] = map->contains(current) ? map->value(current) : current;
		}
	}
	return input;
}

QMap<unsigned char, unsigned char> *character_mapper::map = nullptr;
bool character_mapper::map_state = false;
