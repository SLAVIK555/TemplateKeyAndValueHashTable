// TemplateKeyAndValueHashTable.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <string>
#include <bitset>

template<class K, class V>
class HashTable {
	static const int default_size = 10; // Начальный размер нашей таблицы
	constexpr static const double rehash_size = 0.75; // Коэффициент, при котором произойдет увеличение таблицы

	struct Node {
		V value;
		K key;// По нему хешируем пару ключ - значение и помещаям сюда
		bool state;// Если значение флага state = false, значит элемент массива был удален (deleted)

		// Конструктор структуры
		Node(const V& val, const K& k) : value(val), key(k), state(true) {}
		Node() {}
	};

	Node** arr; // Соответственно в массиве будут хранится структуры Node*

	int size; // Сколько элементов у нас сейчас в массиве (без учета deleted)
	int buffer_size; // Размер самого массива, сколько памяти выделено под хранение нашей таблицы
	int size_all_non_nullptr; // Сколько элементов у нас сейчас в массиве (с учетом deleted)

	std::string GetBits(K key) {// Для представления ключа в формате битсета
		/*Данная функция принимает на вход темплейтный К ключ, после чего
		вереводит его в битсет и затем этот битсет записывает в строку

		Данная функция протестирована со следующими типами данных:
		bool a = true; 00000001;
		char a = 'c'; 01100011;
		int a = 34; 00000000000000000000000000100010;
		float a = 15.5; 01000001011110000000000000000000;
		double a = 16.8; 0000000000000000000000000000000000110011001100110011001100110011;
		Так же функция работает с nullptr; 00000000000000000000000000000000;
		А так же с signed char, unsigned char, char8_t, char16_t, char32_t,
		wchar_t, short, unsigned short, int, unsigned int, long, long,
		unsigned long, unsigned long long, float, double, long double,
		*/
		std::bitset<sizeof(key) * 8> bits;

		int *y, x;
		y = reinterpret_cast<int*>(&key);
		x = *y;
		for (short i = 0; i < sizeof(key) * 8; ++i) {
			bits[i] = x % 2;
			x /= 2;
		}

		return bits.to_string();
	}

	int HashFunctionHorner(const std::string& s, int table_size, int add_table_size)
	{
		int hash_result = 0;
		for (int i = 0; i < s.size(); ++i)
		{
			hash_result = (add_table_size * hash_result + s[i]) % table_size;
		}
		hash_result = (hash_result * 2 + 1) % table_size;
		return hash_result;
	}

	int HashFunction1(K key, int table_size) {
		std::string keyStr = GetBits(key);
		return HashFunctionHorner(keyStr, table_size, table_size - 1);
	}

	int HashFunction2(K key, int table_size) {
		std::string keyStr = GetBits(key);
		return HashFunctionHorner(keyStr, table_size, table_size + 1);
	}

	void Resize() {// Используется тогда, когда в таблице почти не остается свободного места
		std::cout << "Resizing" << std::endl;
		int past_buffer_size = buffer_size;
		buffer_size = buffer_size * 2;
		size_all_non_nullptr = 0;
		size = 0;

		Node** arr2 = new Node *[buffer_size];

		for (int i = 0; i < buffer_size; ++i) {
			arr2[i] = nullptr;
		}

		for (int i = 0; i < past_buffer_size; ++i) {
			if (arr[i] && arr[i]->state) {
				arr2[i] = new Node(arr[i]->value, arr[i]->key);
			}
		}

		std::swap(arr, arr2);

		for (int i = 0; i < past_buffer_size; ++i)
			if (arr2[i])
				delete arr2[i];
		delete[] arr2;
	}

	void Rehash() {// Используется тогда, когда при добавлении нового элемента выясняется, что таблица замусорена как бы удаленными элементами (arr[i]->state == false)
		std::cout << "Rehashing" << std::endl;
		size_all_non_nullptr = 0;
		size = 0;

		Node** arr2 = new Node *[buffer_size];

		for (int i = 0; i < buffer_size; ++i) {
			arr2[i] = nullptr;
		}

		for (int i = 0; i < buffer_size; ++i) {
			if (arr[i] && arr[i]->state) {
				arr2[i] = new Node(arr[i]->value, arr[i]->key);
			}
		}

		std::swap(arr, arr2);

		for (int i = 0; i < buffer_size; ++i)
			if (arr2[i])
				delete arr2[i];
		delete[] arr2;
	}

public:
	HashTable() {
		buffer_size = default_size;
		size = 0;
		size_all_non_nullptr = 0;

		arr = new Node*[buffer_size];
		for (int i = 0; i < buffer_size; ++i)
			arr[i] = nullptr; // заполняем nullptr - то есть если значение отсутствует, и никто раньше по этому адресу не обращался
	}

	~HashTable()
	{
		for (int i = 0; i < buffer_size; ++i)
			if (arr[i])
				delete arr[i];
		delete[] arr;
	}

	bool Add(const K& key, const V& value) {
		if (size + 1 > int(rehash_size * buffer_size))
			Resize(); // Происходит ресайз, так как в таблице осталось мало места
		else if (size_all_non_nullptr > 2 * size)
			Rehash(); // Происходит рехеш, так как слишком много deleted-элементов

		int h1 = HashFunction1(key, buffer_size);
		int h2 = HashFunction2(key, buffer_size);

		int i = 0;
		int first_deleted = -1; // Запоминаем первый подходящий (когда то удаленный) элемент

		while (arr[h1] != nullptr && i < buffer_size)
		{
			if (arr[h1]->value == value && arr[h1]->key == key && arr[h1]->state ||
				arr[h1]->key == key && arr[h1]->state) {
				std::cout << "This value already added" << std::endl;
				return false; // Такой элемент уже есть, а значит его нельзя вставлять повторно
			}
			if (!arr[h1]->state && first_deleted == -1) // Находим место для нового элемента
				first_deleted = h1;

			h1 = (h1 + h2) % buffer_size;
			++i;
		}

		if (first_deleted == -1) // Если не нашлось подходящего места, создаем новый Node
		{
			arr[h1] = new Node(value, key);
			++size_all_non_nullptr; // Так как мы заполнили один пробел, не забываем записать, что это место теперь занято
		}
		else
		{
			arr[first_deleted]->value = value;
			arr[first_deleted]->key = key;
			arr[first_deleted]->state = true;
		}
		++size; // В любом случае мы увеличили количество элементов
		return true;
	}

	bool Remove(const K& key) {
		int h1 = HashFunction1(key, buffer_size); // Значение, отвечающее за начальную позицию
		int h2 = HashFunction2(key, buffer_size); // Значение, ответственное за "шаг" по таблице
		int i = 0;

		while (arr[h1] != nullptr && i < buffer_size || i < buffer_size) {
			if (arr[h1] != nullptr) {
				if (arr[h1]->key == key && arr[h1]->state) {
					arr[h1]->state = false;
					--size;
					std::cout << "key: " << key << " value: " << arr[h1]->value << " deleted" << std::endl;
					return true;
				}
			}
			h1 = (h1 + h2) % buffer_size;
			++i; // Если у нас i >=  buffer_size, значит мы уже обошли абсолютно все ячейки, именно для этого мы считаем i, иначе мы могли бы зациклиться.
		}
		std::cout << "key: " << key << " not found " << std::endl;
		return false;
	}

	bool Find(const K& key) {
		int h1 = HashFunction1(key, buffer_size); // Значение, отвечающее за начальную позицию
		int h2 = HashFunction2(key, buffer_size); // Значение, ответственное за "шаг" по таблице
		int i = 0;

		while (arr[h1] != nullptr && i < buffer_size || i < buffer_size) {
			if (arr[h1] != nullptr) {
				if (arr[h1]->key == key && arr[h1]->state) {
					std::cout << "finded key: " << key << " value: " << arr[h1]->value << std::endl;
					return true; // Такой элемент есть
				}
			}
			h1 = (h1 + h2) % buffer_size;
			++i; // Если у нас i >=  buffer_size, значит мы уже обошли абсолютно все ячейки, именно для этого мы считаем i, иначе мы могли бы зациклиться.
		}
		std::cout << "key: " << key << " not found " << std::endl;
		return false;
	}

	void Show() {// Отображаем таблицу как она есть
		for (int i = 0; i < buffer_size; i++) {
			if (arr[i] != nullptr && arr[i]->state == true) {
				std::cout << "i: " << i << " key: " << arr[i]->key << " value: " << arr[i]->value << std::endl;
			}
			else if (arr[i] != nullptr && arr[i]->state == false) {
				std::cout << "i: " << i << " key: " << arr[i]->key << " value: " << arr[i]->value << " was deleted" << std::endl;
			}
			else if (arr[i] == nullptr) {
				std::cout << "i: " << i << " is fool" << std::endl;
			}
		}
	}
};

int main()
{
	/*Данная хеш таблица записывает в себя пары ключ - значение с уникальностью по ключу
	Пример:
	Add(4, 'f'); // Ok
	Add(4, 'f'); // Не прокатит, такой элемент уже есть в таблице
	Add(5, 'f'); // Ok, В таблице будет 2 записи, одна запись (4 - 'f') под индексом i1,
	другая запись (5 - 'f') под индексом i2
	Add(5, 'g'); // Не прокатит, такой ключ уже занят
	*/
	std::cout << "Hello World!\n";
	HashTable<int, char> ht;
	std::cout << std::endl;
	std::cout << "adding key 1, value a" << std::endl;
	ht.Add(1, 'a');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 2, value b" << std::endl;
	ht.Add(2, 'b');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 1, value a" << std::endl;
	ht.Add(1, 'a');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 4, value c" << std::endl;
	ht.Add(4, 'c');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 7, value k" << std::endl;
	ht.Add(7, 'k');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 5, value o" << std::endl;
	ht.Add(5, 'o');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 3, value l" << std::endl;
	ht.Add(3, 'l');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 3, value f" << std::endl;
	ht.Add(3, 'f');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 9, value f" << std::endl;
	ht.Add(9, 'f');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 8, value g" << std::endl;
	ht.Add(8, 'g');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 7, value r" << std::endl;
	ht.Add(7, 'r');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 6, value e" << std::endl;
	ht.Add(6, 'e');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 0, value m" << std::endl;
	ht.Add(0, 'm');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 10, value m" << std::endl;
	ht.Add(10, 'm');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 11, value n" << std::endl;
	ht.Add(11, 'n');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 13, value j" << std::endl;
	ht.Add(13, 'j');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 17, value t" << std::endl;
	ht.Add(17, 't');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 15, value s" << std::endl;
	ht.Add(15, 's');
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 1" << std::endl;
	ht.Remove(1);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 0" << std::endl;
	ht.Remove(0);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 2" << std::endl;
	ht.Remove(2);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 3" << std::endl;
	ht.Remove(3);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 4" << std::endl;
	ht.Remove(4);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 5" << std::endl;
	ht.Remove(5);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 6" << std::endl;
	ht.Remove(6);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 7" << std::endl;
	ht.Remove(7);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 8" << std::endl;
	ht.Remove(8);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 9" << std::endl;
	ht.Remove(9);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 10" << std::endl;
	ht.Remove(10);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 11" << std::endl;
	ht.Remove(11);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 13" << std::endl;
	ht.Remove(13);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 15" << std::endl;
	ht.Remove(15);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 17" << std::endl;
	ht.Remove(17);
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 1, value y" << std::endl;
	ht.Add(1, 'y');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 2, value x" << std::endl;
	ht.Add(2, 'x');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 1, value a" << std::endl;
	ht.Add(1, 'a');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 4, value z" << std::endl;
	ht.Add(4, 'z');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 7, value q" << std::endl;
	ht.Add(7, 'q');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 5, value o" << std::endl;
	ht.Add(5, 'o');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 3, value w" << std::endl;
	ht.Add(3, 'w');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 3, value f" << std::endl;
	ht.Add(3, 'f');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 9, value f" << std::endl;
	ht.Add(9, 'f');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 8, value u" << std::endl;
	ht.Add(8, 'u');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 7, value r" << std::endl;
	ht.Add(7, 'r');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 6, value v" << std::endl;
	ht.Add(6, 'v');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 0, value m" << std::endl;
	ht.Add(0, 'm');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 10, value m" << std::endl;
	ht.Add(10, 'm');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 11, value p" << std::endl;
	ht.Add(11, 'p');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 13, value j" << std::endl;
	ht.Add(13, 'j');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 17, value t" << std::endl;
	ht.Add(17, 't');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 15, value s" << std::endl;
	ht.Add(15, 's');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 25, value p" << std::endl;
	ht.Add(25, 'p');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 27, value l" << std::endl;
	ht.Add(27, 'l');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 21, value s" << std::endl;
	ht.Add(21, 's');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 35, value a" << std::endl;
	ht.Add(35, 'a');
	ht.Show();

	std::cout << std::endl;
	std::cout << "adding key 16, value r" << std::endl;
	ht.Add(16, 'r');
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 1" << std::endl;
	ht.Remove(1);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 0" << std::endl;
	ht.Remove(0);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 2" << std::endl;
	ht.Remove(2);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 3" << std::endl;
	ht.Remove(3);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 4" << std::endl;
	ht.Remove(4);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 5" << std::endl;
	ht.Remove(5);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 6" << std::endl;
	ht.Remove(6);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 7" << std::endl;
	ht.Remove(7);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 8" << std::endl;
	ht.Remove(8);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 9" << std::endl;
	ht.Remove(9);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 10" << std::endl;
	ht.Remove(10);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 11" << std::endl;
	ht.Remove(11);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 13" << std::endl;
	ht.Remove(13);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 15" << std::endl;
	ht.Remove(15);
	ht.Show();

	std::cout << std::endl;
	std::cout << "deliting key 17" << std::endl;
	ht.Remove(17);
	ht.Show();

	HashTable<float, std::string> fsht;
	std::cout << std::endl;
	fsht.Add(0.1, "asddsa");
	fsht.Show();

	std::cout << std::endl;
	fsht.Add(0.2, "asddshgfda");
	fsht.Show();

	std::cout << std::endl;
	fsht.Add(0.3, "gfdvsfsa");
	fsht.Show();

	std::cout << std::endl;
	fsht.Add(0.4, "aswegfbda");
	fsht.Show();

	std::cout << std::endl;
	fsht.Add(0.5, "aqwertbfvsa");
	fsht.Show();;

	std::cout << std::endl;
	fsht.Add(0.6, "asewfrfvda");
	fsht.Show();

	std::cout << std::endl;
	fsht.Add(0.7, "asddsa");
	fsht.Show();

	std::cout << std::endl;
	fsht.Add(0.8, "adfvsa");
	fsht.Show();

	std::cout << std::endl;
	fsht.Add(0.8, "asddsa");
	fsht.Show();

	std::cout << std::endl;
	fsht.Add(1.0, "awqdefbvdsa");
	fsht.Show();

	std::cout << std::endl;
	fsht.Add(1.1, "avsgfbdvsa");
	fsht.Show();

	std::cout << std::endl;
	fsht.Remove(0.1);
	fsht.Show();

	std::cout << std::endl;
	fsht.Remove(0.2);
	fsht.Show();

	std::cout << std::endl;
	fsht.Remove(0.3);
	fsht.Show();

	std::cout << std::endl;
	fsht.Remove(0.4);
	fsht.Show();

	std::cout << std::endl;
	fsht.Remove(0.5);
	fsht.Show();

	std::cout << std::endl;
	fsht.Remove(0.6);
	fsht.Show();

	std::cout << std::endl;
	fsht.Remove(0.7);
	fsht.Show();

	std::cout << std::endl;
	fsht.Remove(0.8);
	fsht.Show();

	std::cout << std::endl;
	fsht.Remove(0.9);
	fsht.Show();

	std::cout << std::endl;
	fsht.Remove(1.0);
	fsht.Show();

	std::cout << std::endl;
	fsht.Remove(1.1);
	fsht.Show();

	std::cout << std::endl;
	fsht.Add(1.2, "avsgfbdvsa");
	fsht.Show();

}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
