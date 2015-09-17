/*
 * Written by Kevin Neale (C) 2015.
 *
 * Example usage of the Boost C++ libraries.
 *
 */
#include <iostream>
#include <fstream>
#include <map>

#include <boost/foreach.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/random.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

using namespace std;

void get_boost_version()
{
    cout << "Using Boost " << BOOST_VERSION / 100000 << "."     // major version
        << BOOST_VERSION / 100 % 1000 << "."    // minor version
        << BOOST_VERSION % 100  // patch level
        << endl;
}


/******************/
/* RANDOM NUMBERS */
/******************/

//Demonstration of the random number API
void rand_from_uniform_dstrb_test()
{
    //create seed
    unsigned long seed = 12411;

    //produces general pseudo random number series with the Mersenne Twister Algorithm.
    boost::mt19937 rng(seed);

    // Uniform distribution on 1...100.  This is the generation that the random numbers will be generated from.
    // There are all equally likely in the case of a uniform distribution.
    //boost::uniform_int <> hundred(1,100);

    // Normal distribution.
    boost::normal_distribution <> hundred(50, 10);

    //Connects the distribution with random series
    //boost::variate_generator <boost::mt19937&, boost::uniform_int <> > unhundred(rng,hundred);
    boost::variate_generator < boost::mt19937 &,
        boost::normal_distribution <> >unhundred(rng, hundred);

    cout << "\nRandom number generation example (Normal Distribution): \n";

    int i = 0, n = 10;

    for (i = 0; i < n; i++) {
        cout << unhundred() << std::endl;
    }
}


/*****************/
/* SERIALIZATION */
/*****************/

/* 
 * Demonstration of the serialization API.
 * It serializes data from a map object to a binary file.
 * This data is then deserialized to a second map object.
 *
 * Used in situations where the population of a object might take a long time.
 * In this event, you could serialize and deserialize to a file instead of 
 * reprocessing the original input. 
 *
 * Also useful for deconstructing objects, sending across a network and
 * reconstruction.  Useful for producing things similar to core dumps for
 * debugging purposes.
 *
 */
void serialization_api_test()
{
    map < int, string > mymap1;
    map < int, string > mymap2;

    mymap1.insert(pair < int, std::string > (1, "Hello, "));
    mymap1.insert(pair < int, std::string > (2, "this "));
    mymap1.insert(pair < int, std::string > (3, "is "));
    mymap1.insert(pair < int, std::string > (4, "a "));
    mymap1.insert(pair < int, std::string > (5, "message."));

    cout << "\nSerialization example: \n";

    pair < int, string > x, y;

    BOOST_FOREACH(x,
                  mymap1) cout << "Key: " << x.first << ", Value: " << x.
        second << std::endl;

    ofstream ostr("map.dat", ios::binary);

    boost::archive::binary_oarchive oa(ostr);

    oa << mymap1;

    ostr.close();

    ifstream istr("map.dat", ios::binary);

    boost::archive::binary_iarchive ia(istr);

    //Restore the data from a previosuly serialised object to a new object.
    ia >> mymap2;

    BOOST_FOREACH(y,
                  mymap2) cout << "Key: " << y.first << ", Value: " << y.
        second << std::endl;
}


/******************/
/* SHARED POINTER */
/******************/

//A test class which dynamically allocates memory which is at risk not being freed!
class TestClass1 {
  public:
    TestClass1(int value) {
        m_value = value;
        m_data = new int[100];
    };
    ~TestClass1() {
        cout << "Destructor called!\n";
        delete[]m_data;
    };

  private:
    int m_value;
    int *m_data;
};

/* 
 * Shared pointer example. 
 * A simple class that allocates some memory on construction.
 * It's destructor deallocates it.
 *
 * With a smart/shared pointer, when the last reference to it goes out of scope, 
 * it will call the class's destructor.
*/
boost::shared_ptr < TestClass1 > shared_pointer_test1()
{
    boost::shared_ptr < TestClass1 > bptr_tc11(new TestClass1(1));
    boost::shared_ptr < TestClass1 > bptr_tc12(new TestClass1(2));
    boost::shared_ptr < TestClass1 > bptr_tc13(new TestClass1(3));
    return bptr_tc13;
}

void shared_pointer_test()
{
    cout << "\nShared pointer example: \n";
    boost::shared_ptr < TestClass1 > bptr = shared_pointer_test1();

    cout <<
        "We have a copy of the shared pointer. Therefore it will go out of ";
    cout << "scope when 'shared_pointer_test' returns instead!\n";

    return;
}


/******************/
/* MULTITHREADING */
/******************/

/*
 * A simple class obeying the singleton design pattern.
 *
 * Demonstrates the usage of a 'boost mutex' and a boost 'scoped lock' to make 'GetInstance'
 *  threadsafe.  Well to a degree...
 * 
*/
class TestClass2 {
  public:
    ~TestClass2() {
        cout << "Destructor called for singleton!\n";
    };                          //Privet destructor

    static TestClass2 & GetInstance() {
        static boost::mutex mutex;

        //A scoped lock, so we don't have to make explicit 'lock' and 'unlock' calls.
        static boost::lock_guard < boost::mutex > guard(mutex);

        //Within a multithreaded environment, this could have been a problem
        static TestClass2 instance;
        return instance;
    }

  private:
    TestClass2() {
        cout << "Constructor called for singleton!\n";
    };                          //Private constructor

    TestClass2(const TestClass2 &);     // Prevent copy-construction
    TestClass2 & operator=(const TestClass2 &); // Prevent assignment


};

//TestClass1::m_instance = NULL;

//main function for a boost thread;
void thread_main_1()
{
    while (1) {
        cout << "Thread 1 - main body\n";
        sleep(5);
        TestClass2 & instance = TestClass2::GetInstance();
    }

    return;
}

/* 
 * Refer to here, for much more detail on multithreading with boost:
 * http://www.boost.org/doc/libs/1_58_0/doc/html/thread/synchronization.html
 *
 * The aim here it to create 2 competing threads which attempt to acquire a shared resource 
 * (which is a singleton by design).  
 * 1) A worker thread will periodically try to obtain an instance of the resource.
 * 2) The main thread will also try to obtain an instance of the resource.
 *
 * Only one of instance of the singleton should ever be obtained and subsequently only one 
 * 'destruction' event should occur.  This can be seen to be true via the debug traces for the 
 * constructor and destructor respectively.
 */
void multithreading_test()
{
    boost::thread thread1(thread_main_1);       // start concurrent execution of bankAgent
    thread1.detach();

    cout << "\nMultithreading example: \n";

    int count = 1;

    while (count <= 5) {
        cout << "Main Thread - main body\n";
        sleep(5);
        TestClass2 & instance = TestClass2::GetInstance();
        count++;
    }

    return;
}


/********/
/* MAIN */
/********/
int main()
{
    get_boost_version();
    serialization_api_test();
    rand_from_uniform_dstrb_test();
    shared_pointer_test();
    multithreading_test();

    return 0;
}
