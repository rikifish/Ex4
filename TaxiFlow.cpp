#include "TaxiFlow.h"
BOOST_CLASS_EXPORT_GUID(Node, "Node");
BOOST_CLASS_EXPORT_GUID(GridPt, "GridPt");
BOOST_CLASS_EXPORT_GUID(Taxi, "Taxi");
BOOST_CLASS_EXPORT_GUID(Luxury, "Luxury");

int main(int argc, char *argv[]) {
    // checks we got a correct number of args.
    if (argc < 2) {
        return 0;
    }
    //.initializes the servers socket.
    Socket* socket = new Udp(1, atoi(argv[1]));
    TaxiFlow flow = TaxiFlow(socket);
    // gets the input from the user and runs the taxi center.
    flow.getInput();
    return 0;
}

TaxiFlow::TaxiFlow(Socket* socket1) {
    socket = socket1;
    socket->initialize();
}

TaxiFlow::~TaxiFlow() {
    delete socket;
}

void TaxiFlow::getInput() {
    int x, y, numOfObs;
    // will read the ','.
    char comma;
    Point mapSize;
    // reads the map size.
    cin >> skipws >> x >> y;
    mapSize = Point(x,y);
    Map* map = new Map(mapSize);
    // reads the number of obstacles.
    cin >> skipws >> numOfObs;
    // reads the obstacle.
    for (int i = 0; i < numOfObs; i++) {
        cin >> x >> comma >> y;
        GridPt obs = GridPt(Point(x,y));
        //obstacles.push_back(&obs);
        map->addObstacle(&obs);
    }
    // creates the taxi center with the given map.
    center = TaxiCenter(map);
    // tuns the commands.
    run();
    delete map;
}

void TaxiFlow::run() {
    // the command from the user.
    int command;
    do {
        // reads the command.
        cin >> command;
        switch (command) {
            // for '1' -adds drivers.
            case 1:
                addDrivers();
                break;
            // for '2' -adds a trip.
            case 2:
                addTrip();
                break;
            // for '3' -adds a cab.
            case 3:
                addCab();
                break;
            // for '4' - gets a drivers location.
            case 4:
                getDriverLocation();
                break;
            case 7:
                closeClients();
            // for '9' - drives the cars.
            case 9:
                drive();
                break;
            default:
                break;
        }
    // for '7' - end program.
    } while (command != 7);
}

void TaxiFlow::addDrivers() {
    int numDrivers;
    // gets the number of drivers from the user.
    cin >> numDrivers;
    char buffer[1000];
    for (int i = 0; i < numDrivers; i++) {
        // get the driver.
        socket->receiveData(buffer, sizeof(buffer));
        Driver* driver;
        // gets the driver from client.
        boost::iostreams::basic_array_source<char> device(buffer, sizeof(buffer));
        boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(device);
        // deserizlizes the driver from client.
        boost::archive::binary_iarchive ia(s);
        ia >> driver;
        // sets the drivers map.
        driver->setMap(center.getMap());
        //assigns the driver his cab.
        center.assignCab(driver);
        //sends the cab to the driver.
        Taxi* t = driver->getCab();
        std::string serial_str2;
        boost::iostreams::back_insert_device<std::string> inserter2(serial_str2);
        boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> > s2(inserter2);
        boost::archive::binary_oarchive oa2(s2);
        // serilizes the taxi.
        oa2 << t;
        // flush the stream to finish writing into the buffer
        s2.flush();
        // sends the taxi.
        socket->sendData(serial_str2);
        // adds the driver to the taxi center.
        center.addDriver(driver);
    }
}

void TaxiFlow::addTrip() {
    int id, xStart, yStart, xEnd, yEnd, numPassengers, startTime;
    double tariff;
    // skips the punctuation marks.
    char skip;
    // gets the trip's details from the user.
    cin >> id >> skip >> xStart >> skip >> yStart >> skip >> xEnd
        >> skip >> yEnd >> skip >> numPassengers >> skip >> tariff >> skip >> startTime;
    // gets the starting point from the map.
    Point* start = new Point(xStart,yStart);//center.getMap()->getPoint(Point(xStart,yStart));
    // gets the ending point from the map.
    Point* end = new Point(xEnd,yEnd);//center.getMap()->getPoint(Point(xEnd,yEnd));
    // creates the new trip.
    Trip* trip = new Trip(id, start, end, numPassengers, tariff, startTime);
    // adds the trip to the center.
    center.addTrip(trip);
}

void TaxiFlow::addCab() {
    int id, type;
    char manufacturerSign, colorSign;
    // skips the punctuation marks.
    char skip;
    MANUFACTURER manufacturer;
    COLOR color;
    Taxi* taxi;
    // gets the cab's details from the user.
    cin >> id >> skip >> type >> skip >> manufacturerSign >> skip >> colorSign;
    // assigns the cars manufacturer.
    switch (manufacturerSign) {
        case 'H':
            manufacturer = HONDA;
            break;
        case 'S':
            manufacturer = SUBARO;
            break;
        case 'T':
            manufacturer = TESLA;
            break;
        case 'F':
            manufacturer = FIAT;
            break;
        default:
            break;
    }
    // assigns the cars color.
    switch (colorSign) {
        case 'R':
            color = RED;
            break;
        case 'B':
            color = BLUE;
            break;
        case 'G':
            color = GREEN;
            break;
        case 'P':
            color = PINK;
            break;
        case 'W':
            color = WHITE;
            break;
        default:
            break;
    }
    // assigns the taxis type.
    if (type == 1) {
        taxi = new Taxi(id, manufacturer, color);
    } else if (type == 2) {
        taxi = new Luxury(id, manufacturer, color);
    } else {
        return;
    }
    // adds the taxi.
    center.addTaxi(taxi);
}

void TaxiFlow::getDriverLocation() {
    int id;
    // gets the drivers id.
    cin >> id;
    // gets the drivers from the center.
    vector<Driver*> drivers = center.getDrivers();
    // looks for the driver.
    for (int i = 0; i < drivers.size(); i++) {
        if (id == drivers.at(i)->getId()) {
            // prints the location of the given driver.
            cout << *((GridPt*)(drivers.at(i)->getLocation())) << endl;
            break;
        }
    }
}

void TaxiFlow::drive() {
    // increases the time.
    center.setTime();
    // sends the drivers to drive.
    for (int i = 0; i < center.getDrivers().size(); i++) {
        if (center.getDrivers().at(i)->isDriving()) {
            // tells the client to be prepared to drive.
            socket->sendData("go");
            // drives the car.
            center.getDrivers().at(i)->drive();
            // sends the new location to client.
            std::string serial_str1;
            boost::iostreams::back_insert_device<std::string> inserter1(serial_str1);
            boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> > s1(inserter1);
            boost::archive::binary_oarchive oa1(s1);
            GridPt* newLocation = new GridPt(center.getDrivers().at(i)->getLocation()->getPt());
            // serilizes the new location.
            oa1 << newLocation;
            // flush the stream to finish writing into the buffer
            s1.flush();
            // sends the data.
            socket->sendData(serial_str1);
            delete newLocation;
        }
    }
    // assign trips to the drivers who are ready for them.
    center.sendTaxi();
    for (int i = 0; i < center.getDrivers().size(); i++) {
        // if the driver just got a new trip.
        if (center.getDrivers().at(i)->gotNewTrip()) {
            // tells the client to be prepared to get a new trip.
            socket->sendData("trip");
            // sends the trip.
            std::string serial_str;
            boost::iostreams::back_insert_device<std::string> inserter(serial_str);
            boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> > s(inserter);
            boost::archive::binary_oarchive oa(s);
            Trip* newTrip = center.getDrivers().at(i)->getTrip();
            // serilizes the trip.
            oa << newTrip;
            // flush the stream to finish writing into the buffer.
            s.flush();
            // sentd the new trip.
            socket->sendData(serial_str);
            // deletes the trip from the taxiCenter.
            delete (center.getDrivers().at(i)->getTrip());
            center.getDrivers().at(i)->setNewTrip();
        }
    }
}

void TaxiFlow::closeClients() {
    // tells the client he can close now.
    socket->sendData("exit");
}
