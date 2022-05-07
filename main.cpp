#include <SFML/Graphics.hpp>
#include <vector>
#include <array>
#include <list>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <mutex>

constexpr int COLS = 300;
constexpr int ROWS = 280;
constexpr int WINDOW_WIDTH = 1200;
constexpr int WINDOW_HEIGHT = 800;

constexpr int REFRESH_RATE = 2000; // refresh rate in milliseconds

constexpr int GNUS_AT_START = 20;
constexpr int GNU_DURATION = 100;
constexpr int LIONS_AT_START = 20;
constexpr int LION_DURATION = 200;
constexpr int GRAS_AT_START = 1000;

std::mutex mtx;

bool is_running = true;

template <typename T>
class Matrix
{
public:
    Matrix<T>( const int cols, const int rows )
        : m_cols{cols}, m_rows{rows}, m_data( cols * rows )
    {}

    T * get_cell( int col, int row )
    {
        return m_data[ col + row * m_cols ];
    }

    const T * get_cell( int col, int row ) const
    {
        return  m_data[col+row*m_cols];
    }

    void set_cell( T * t )
    {
        m_data[ t->x + t->y * m_cols ] = t;
    }

    sf::Vector2i crd( int i ) const
    {
        return sf::Vector2i( i%m_cols, i/m_rows );
    }
    bool is_cell_valid( int col, int row ) const
    {
        return m_data[ col + row * m_cols ] == nullptr ? false : true;
    }

    bool is_cell_empty( int col, int row ) const
    {
        return ! is_cell_valid( col, row );
    }

    void clear_cell( int col, int row )
    {
        m_data[ col + row * m_cols ] = nullptr;
    }

    int cols() const
    {
        return m_cols;
    }
    int rows() const
    {
        return m_rows;
    }
    int size() const
    {
        return m_data.size();
    }

    std::array<sf::Vector2<unsigned int>, 8> neighbor_cells( int x, int y )
    {

        typedef sf::Vector2<unsigned int> V;

        std::array<V, 8> arr;           // +---------------+ //
        arr[0] = V( x, y-1 );           // |-1-1  0-1  1-1 \ //
        arr[1] = V( x+1, y-1 );         // |-1 0  0 0  1 0 \ //
        arr[2] = V( x+1, y );           // |-1 1  0 1  1 1 \ //
        arr[3] = V( x+1, y+1 );         // +---------------+ //
        arr[4] = V( x, y+1 );
        arr[5] = V( x-1, y+1 );
        arr[6] = V( x-1, y );
        arr[7] = V( x-1, y-1 );

        for( auto & v : arr )
        {
            v.x %= m_cols;
            v.y %= m_rows;
        }
        return arr;
    }
    std::vector<sf::Vector2<unsigned int>> empty_neighbor_cells( int x, int y )
    {
        auto arr = neighbor_cells( x, y );
        std::vector<sf::Vector2<unsigned int>> v;
        for( auto const & cell : arr )
        {
            if( is_cell_empty( cell.x, cell.y ) )
                v.push_back( cell );
        }
        return v;
    }


private:
    int m_cols, m_rows;
    std::vector<T *> m_data;
};

struct Life
{

    enum Kind { Herbivore, Carnivore, Vegetable, None };
    enum Name { Lion, Gnu, Grass };

    Kind kind; // static
    Name name;  // static
    unsigned int x, y;
    int health;
    unsigned int spawnsize; // static
    unsigned int speed;  // static
    sf::Color color; // static
    bool is_dead;

    Life( unsigned int col = 0, unsigned int row = 0 )
        : x{col}, y{row}, is_dead{false}
    {}

};

struct Lion : public Life
{
    Lion( unsigned int col = 0, unsigned int row = 0 )
        : Life{col, row}
    {
        name = Life::Gnu;
        kind = Life::Carnivore;
        color = sf::Color::Yellow;
        spawnsize = LION_DURATION + 100;
        speed = 1;
        health = rand() % LION_DURATION;
    }
};
struct Gnu : public Life
{
    Gnu( unsigned int col = 0, unsigned int row = 0 )
        : Life{col, row}
    {
        name = Life::Gnu;
        kind = Life::Herbivore;
        color = sf::Color::Black;
        spawnsize = GNU_DURATION + 40;
        speed = 1;
        health = rand() % GNU_DURATION;
    }
};
struct Grass : public Life
{
    Grass( unsigned int col = 0, unsigned int row = 0 )
        : Life{col, row}
    {
        name = Life::Grass;
        kind = Life::Vegetable;
        color = sf::Color::Green;
        spawnsize = 40;
        speed = 0;
        health = rand() % 20;
    }
};

std::ostream & operator<<( std::ostream & os, const Life::Kind kind )
{
    switch( kind )
    {
        case Life::Carnivore :
            os << "Carnivore";
            break;
        case Life::Herbivore :
            os << "Herbivore";
            break;
        case Life::Vegetable :
            os << "Vegetable";
            break;
        case Life::None :
            os << "None";
            break;
        default:
            os << "unknown";
    }
    return os;
}
std::ostream & operator<<( std::ostream & os, const Life::Name name )
{
    switch( name )
    {
        case Life::Gnu :
            os << "gnu";
            break;
        case Life::Lion :
            os << "lion";
            break;
        case Life::Grass :
            os << "grass";
            break;
        default:
            os << "unknown";
    }
    return os;
}

std::ostream & operator<<( std::ostream & o, const Life *& l )
{
    return o << '(' << l->x << "," << l->y << ") "
           << l->kind << ' ' << l->name << ' ' << l->health << '\n';
}


template <typename T>
std::ostream & operator<<( std::ostream & o, const Matrix<T> & m )
{
    for( int row = 0; row < m.rows(); ++row )
        for( int col = 0; col < m.cols(); ++col )
        {
            o << col << "," << row << " ";
            if ( m.is_cell_valid( col, row ) ) o << m.get_cell( col, row );
            else o << "nullptr";
            o << '\n';
        }
    return o;
}

void test()
{
    Matrix<Life> m( 10, 10 );
    Life * life;
    for( int i = 0; i < 10; ++i ) m.set_cell( life );

    for( int row = 0; row < 10; ++row )
        for( int col = 0; col < 10; ++col )
        {
            if( m.is_cell_valid( col, row ) ) std::cerr << m.get_cell( col, row );
        }
    std::cerr << m << '\n';
}

template <typename T>
class Game
{
public:
    Game<T>( const int cols, const int rows, sf::RenderWindow & win )
        : m_matrix{cols, rows}, m_window( win )
    {
        setup();
    }

    void setup()
    {
        sf::Vector2u win_size = m_window.getSize();
        m_win_w = win_size.x;
        m_win_h = win_size.y;
        unsigned int x, y;
        Life * life = nullptr;

        // set grass
        for( int i = 0; i < GRAS_AT_START; ++i )
        {
            x = std::rand() % m_matrix.cols();
            y = std::rand() % m_matrix.rows();
            if( m_matrix.is_cell_empty( x, y ) )
            {
                life = new Grass( x, y );
                m_list.push_back( life );
                m_matrix.set_cell( life );
            }
        }

        // set gnus
        for( int i = 0; i < GNUS_AT_START; ++i )
        {
            x = std::rand() % m_matrix.cols();
            y = std::rand() % m_matrix.rows();
            if( m_matrix.is_cell_empty( x, y ) )
            {
                life = new Gnu( x, y );
                m_list.push_back( life );
                m_matrix.set_cell( life );
            }
        }

        // set lions
        for( int i = 0; i < LIONS_AT_START; ++i )
        {
            x = std::rand() % m_matrix.cols();
            y = std::rand() % m_matrix.rows();
            if( m_matrix.is_cell_empty( x, y ) )
            {
                life = new Lion( x, y );
                m_list.push_back( life );
                m_matrix.set_cell( life );
            }
        }
    }

    ~Game()
    {
        for( auto life : m_list ) delete life;
    }


    void update()
    {
        eat();
        spawn();
        move();
        remove_dead_lives();
        seed_fly();
    }

    void remove_dead_lives()
    {
        for( auto it = m_list.begin(); it != m_list.end(); )
        {
            if( ( *it )->is_dead )
            {
                m_matrix.clear_cell( ( *it )->x, ( *it )->y );
                delete *it;
                it = m_list.erase( it );
            }
            else
                ++it;
        }
    }

    void seed_fly()
    {
        for( int i = 0; i < 3; ++i )
        {
            int col = std::rand() % m_matrix.cols();
            int row = std::rand() % m_matrix.rows();
            if( m_matrix.is_cell_empty( col, row ) )
            {
                Life * life = new Grass( col, row );
                m_list.push_back( life );
                m_matrix.set_cell( life );
            }
        }
    }

    void move()
    {
        for( auto life : m_list )
        {
            for( int i = 0; i < life->speed; ++i )
            {
                --( life->health );
                if( life->health <= 0 ) life->is_dead = true;

                auto arr = m_matrix.empty_neighbor_cells( life->x, life->y );

                if( arr.size() != 0 )
                {
                    int r = std::rand() % arr.size();
                    m_matrix.clear_cell( life->x, life->y );
                    life->x = arr[r].x;
                    life->y = arr[r].y;
                    m_matrix.set_cell( life );

                }
            }
        }
    }

    void eat()
    {
        for( auto life : m_list )
        {
            if( life->kind == Life::None )
                continue;

            if( life->kind == Life::Vegetable )
            {
                ++( life->health );
                continue;
            }


            auto arr = m_matrix.neighbor_cells( life->x, life->y );

            if( life->kind == Life::Herbivore )
            {
                for( auto & crd : arr )
                {
                    T * p = m_matrix.get_cell( crd.x, crd.y );
                    if( p != nullptr && p->kind == Life::Vegetable )
                    {
                        //std::cerr << life->name << " browses\n";
                        life->health += p->health;
                        p->is_dead = true;
                        m_matrix.clear_cell( life->x, life->y );
                        life->x = p->x, life->y = p->y;
                        m_matrix.set_cell( life );
                        break;
                    }
                }
            }
            else if( life->kind == Life::Carnivore )
            {
                for( auto & crd : arr )
                {
                    T * p = m_matrix.get_cell( crd.x, crd.y );
                    if( p != nullptr && p->kind == Life::Herbivore )
                    {
                        //std::cerr << life->name << " eats\n";
                        life->health += p->health;
                        p->is_dead = true;
                        m_matrix.clear_cell( life->x, life->y );
                        life->x = p->x, life->y = p->y;
                        m_matrix.set_cell( life );
                        break;
                    }
                }
            }
        }
    }

    void spawn()
    {
        for( auto life : m_list )
        {
            if( life->health < life->spawnsize )
            {
                continue;
            }
            auto arr = m_matrix.empty_neighbor_cells( life->x, life->y );

            if( arr.size() )
            {
                int r = std::rand() % arr.size();

                life->health /= 2;
                Life * child = nullptr;
                Life::Name name = life->name;
                if( name == Life::Lion ) child = new Lion();
                else if( name == Life::Gnu ) child = new Gnu();
                else if( name == Life::Grass ) child = new Grass();

                if( child != nullptr )
                {
                    child->x = arr[r].x;
                    child->y = arr[r].y;
                    child->kind = life->kind;
                    child->name = life->name;
                    child->health = life->health;
                    child->spawnsize = life->spawnsize;
                    child->speed = life->speed;
                    child->color = life->color;
                    child->is_dead = life->is_dead;

                    m_list.push_back( child );
                    m_matrix.set_cell( child );
                }
            }
        }
    }


    void test()
    {
        int lions = 0;
        int grass = 0;
        int gnus = 0;
        for( const auto life : m_list )
        {
            std::cerr << life;
        }
        std::cerr << '\n';
    }

    void draw()
    {
        m_window.clear( sf::Color( 0x99, 0x99, 0x99 ) );

        for( auto life : m_list )
        {
            draw_life( life );
        }
        m_window.display();
    }
    void draw_life( Life * life )
    {
        static sf::RectangleShape r;
        r.setSize(
            sf::Vector2f( ( float )m_win_w / m_matrix.cols()
                          , ( float )m_win_w / m_matrix.rows() )
        );
        r.setPosition( r.getSize().x * life->x, r.getSize().y * life->y );
        r.setFillColor( life->color );
        m_window.draw( r );
    }

    void run()
    {
        while( m_window.isOpen() )
        {
            //test();
            // handle_input(); // moved to its own thread
            update();
            draw();
            std::this_thread::sleep_for( std::chrono::milliseconds( REFRESH_RATE ) );
        }
    }

    sf::RenderWindow & get_win()
    {
        return m_window;
    }

private:
    int m_win_w, m_win_h;
    Matrix<T> m_matrix;
    std::list<T *> m_list;
    sf::RenderWindow & m_window;
};

void handle_input( sf::RenderWindow & win )
{
    sf::Event event;
    while ( is_running )
    {
        win.waitEvent( event );
        if( event.type == sf::Event::Closed )
        {
            mtx.lock();
            is_running = false;
            mtx.unlock();
            win.close();
        }
        else if ( event.type == sf::Event::MouseButtonPressed )
        {
            auto pos = sf::Mouse::getPosition();
            std::cerr << '(' << pos.x << ',' << pos.y << ") ";
         }
    }
}

// invoked in main thread
void run_game( Game<Life> & game )
{
    while( is_running ) game.run();
}

sf::RenderWindow win( sf::VideoMode( WINDOW_WIDTH, WINDOW_HEIGHT ), "Habitat" );
Game<Life> game( COLS, ROWS, win );

int main( int argc, char ** argv )
{
    if( argc > 1 && strcmp( "-test", argv[1] ) == 0 )
    {
        test();
        return 0;
    }

    std::thread input_thread( handle_input, std::ref( win ) );
    while( is_running ) game.run();
    input_thread.join();
}

