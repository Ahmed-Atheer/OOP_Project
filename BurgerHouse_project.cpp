#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
using namespace std;

// ============ Ingredient ============
class Ingredient {
private:
    string name;
    double cost, qty;
public:
    Ingredient(string n, double c, double q) : name(n), cost(c), qty(q) {}
    string getName() const { return name; }
    double getCost() const { return cost; }
    double getQty() const { return qty; }
    void consume(double a) { qty -= a; }
};

// ============ MenuItem ============
class MenuItem {
private:
    struct Part { Ingredient* ing; double amount; };
    string name;
    double price;
    int sold;
    vector<Part> parts;
public:
    MenuItem(string n, double p) : name(n), price(p), sold(0) {}
    void addIngredient(Ingredient* i, double a) { parts.push_back({i, a}); }

    string getName() const { return name; }
    double getPrice() const { return price; }
    int getSold() const { return sold; }

    double getCost() const {
        double c = 0;
        for (auto& p : parts) c += p.ing->getCost() * p.amount;
        return c;
    }
    bool canPrepare(int q) const {
        for (auto& p : parts)
            if (p.ing->getQty() < p.amount * q) return false;
        return true;
    }
    void prepare(int q) {
        for (auto& p : parts) p.ing->consume(p.amount * q);
        sold += q;
    }
    void print(int i) const {
        cout << i << ". " << left << setw(16) << name << price << " $  [";
        for (auto& p : parts) cout << p.ing->getName() << " ";
        cout << "]\n";
    }
};

// ============ Invoice ============
class Invoice {
private:
    static int counter;
    int id;
    double discountRate, taxRate;
    vector<string> names;
    vector<int> qtys;
    vector<double> prices;
public:
    Invoice(double tax = 0.05) : id(++counter), discountRate(0), taxRate(tax) {}
    ~Invoice() {}

    void addLine(string n, int q, double p) {
        names.push_back(n); qtys.push_back(q); prices.push_back(p);
    }
    void setDiscountRate(double r) { discountRate = r; }
    bool isEmpty() const { return names.empty(); }

    double subtotal() const {
        double s = 0;
        for (size_t i = 0; i < prices.size(); i++) s += prices[i] * qtys[i];
        return s;
    }
    double discount() const { return subtotal() * discountRate; }
    double tax() const { return (subtotal() - discount()) * taxRate; }
    double total() const { return subtotal() - discount() + tax(); }

    void print() const {
        cout << fixed << setprecision(2)
             << "\n====== INVOICE #" << id << " ======\n";
        for (size_t i = 0; i < names.size(); i++)
            cout << left << setw(16) << names[i] << "x" << qtys[i]
                 << "  " << prices[i] * qtys[i] << endl;
        cout << "-------------------------\nSubtotal: " << subtotal() << endl;
        if (discountRate > 0)
            cout << "Discount: -" << discount() << endl;
        cout << "Tax     : " << tax() << "\nTOTAL   : " << total()
             << "\n=========================\n";
    }
};
int Invoice::counter = 0;

// ============ Restaurant ============
class Restaurant {
private:
    string name;
    vector<Ingredient*> stock;
    vector<MenuItem*> menu;
    double sales = 0, cost = 0, discounts = 0;
    int invoices = 0;
public:
    Restaurant(string n) : name(n) {}
    ~Restaurant() {
        for (auto m : menu) delete m;
        for (auto s : stock) delete s;
        cout << "Restaurant closed. Memory released.\n";
    }

    Ingredient* addIngredient(string n, double c, double q) {
        stock.push_back(new Ingredient(n, c, q));
        return stock.back();
    }
    MenuItem* addMenuItem(string n, double p) {
        menu.push_back(new MenuItem(n, p));
        return menu.back();
    }

    void showMenu() const {
        cout << "\n------- MENU -------\n";
        for (size_t i = 0; i < menu.size(); i++) menu[i]->print(i + 1);
    }
    void showStock() const {
        cout << "\n------- STOCK -------\n";
        for (auto s : stock)
            cout << left << setw(15) << s->getName() << s->getQty() << endl;
    }

    void takeOrder() {
        Invoice inv;
        double orderCost = 0;
        char more;
        do {
            showMenu();
            int c, q;
            cout << "Item number: "; cin >> c;
            cout << "Quantity: ";    cin >> q;
            if (c < 1 || c > (int)menu.size() || q <= 0)
                cout << "Invalid input.\n";
            else if (!menu[c - 1]->canPrepare(q))
                cout << "Not enough ingredients!\n";
            else {
                menu[c - 1]->prepare(q);
                inv.addLine(menu[c - 1]->getName(), q, menu[c - 1]->getPrice());
                orderCost += menu[c - 1]->getCost() * q;
            }
            cout << "Add more? (y/n): "; cin >> more;
        } while (more == 'y' || more == 'Y');

        if (inv.isEmpty()) { cout << "Order cancelled.\n"; return; }

        if (inv.subtotal() >= 50) {
            char d;
            cout << "Eligible for 10% discount. Apply? (y/n): "; cin >> d;
            if (d == 'y' || d == 'Y') inv.setDiscountRate(0.10);
        }
        inv.print();
        sales += inv.subtotal() - inv.discount();
        discounts += inv.discount();
        cost += orderCost;
        invoices++;
    }

    void report() const {
        cout << fixed << setprecision(2)
             << "\n======= DAILY REPORT =======\n"
             << "Invoices : " << invoices << "\nSales    : " << sales
             << "\nDiscounts: " << discounts << "\nCost     : " << cost
             << "\nProfit   : " << sales - cost << endl;
        MenuItem* best = nullptr;
        for (auto m : menu)
            if (!best || m->getSold() > best->getSold()) best = m;
        if (best && best->getSold() > 0)
            cout << "Best seller: " << best->getName()
                 << " (" << best->getSold() << ")\n";
        cout << "============================\n";
    }
};

// ============ main ============
int main() {
    Restaurant r("Burger House");
    auto bun = r.addIngredient("Bun", 0.30, 100);
    auto beef = r.addIngredient("Beef", 1.50, 80);
    auto chicken = r.addIngredient("Chicken", 1.20, 60);
    auto cheese = r.addIngredient("Cheese", 0.40, 100);
    auto lettuce = r.addIngredient("Lettuce", 0.10, 100);
    auto potato = r.addIngredient("Potato", 0.50, 100);

    auto b1 = r.addMenuItem("Classic Burger", 6.50);
    b1->addIngredient(bun, 1); b1->addIngredient(beef, 1); b1->addIngredient(lettuce, 1);
    auto b2 = r.addMenuItem("Cheese Burger", 7.50);
    b2->addIngredient(bun, 1); b2->addIngredient(beef, 1); b2->addIngredient(cheese, 1);
    auto b3 = r.addMenuItem("Chicken Burger", 7.00);
    b3->addIngredient(bun, 1); b3->addIngredient(chicken, 1); b3->addIngredient(lettuce, 1);
    auto f = r.addMenuItem("French Fries", 3.00);
    f->addIngredient(potato, 2);

    int ch;
    do {
        cout << "\n1.Menu 2.New order 3.Stock 4.Report 0.Exit\nChoice: ";
        cin >> ch;
        switch (ch) {
            case 1: r.showMenu(); break;
            case 2: r.takeOrder(); break;
            case 3: r.showStock(); break;
            case 4: r.report(); break;
        }
    } while (ch != 0);
}
