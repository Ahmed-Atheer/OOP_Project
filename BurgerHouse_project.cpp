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
    void restore(double a) { qty += a; }   // NEW: give ingredients back
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
    // NEW: undo prepare(q) -> ingredients return to stock, sold decreases
    void cancel(int q) {
        for (int i = 0; i < (int)parts.size(); i++)
            parts[i].ing->restore(parts[i].amount * q);
        sold -= q;
    }

    // ---- menu line ----
    void print(int i) const {
        cout << fixed << setprecision(2)
             << " " << left << setw(4) << i
             << setw(18) << name
             << right << setw(6) << price << " $   " << left;
        for (int k = 0; k < (int)parts.size(); k++) {
            cout << parts[k].ing->getName();
            if (k < (int)parts.size() - 1) cout << ", ";
        }
        cout << endl;
    }
};

// ============ Invoice (= one saved order) ============
class Invoice {
private:
    int id;
    double discountRate, taxRate;
    vector<string> names;
    vector<int> qtys;
    vector<double> prices;
public:
    Invoice(int i, double tax = 0.05) : id(i), discountRate(0), taxRate(tax) {}

    // ---- order lines ----
    void addLine(string n, int q, double p) {
        for (int i = 0; i < (int)names.size(); i++)
            if (names[i] == n) { qtys[i] += q; return; }   // same item: just add quantity
        names.push_back(n); qtys.push_back(q); prices.push_back(p);
    }
    void setQty(int i, int q) { qtys[i] = q; }
    void removeLine(int i) {
        names.erase(names.begin() + i);
        qtys.erase(qtys.begin() + i);
        prices.erase(prices.begin() + i);
    }

    // ---- getters ----
    int getId() const { return id; }
    int getLinesCount() const { return names.size(); }
    string getName(int i) const { return names[i]; }
    int getQty(int i) const { return qtys[i]; }
    double getDiscountRate() const { return discountRate; }
    void setDiscountRate(double r) { discountRate = r; }
    bool isEmpty() const { return names.empty(); }

    // ---- calculations ----
    double subtotal() const {
        double s = 0;
        for (int i = 0; i < (int)prices.size(); i++) s += prices[i] * qtys[i];
        return s;
    }
    double discount() const { return subtotal() * discountRate; }
    double tax() const { return (subtotal() - discount()) * taxRate; }
    double total() const { return subtotal() - discount() + tax(); }

    void print() const {
        cout << fixed << setprecision(2);
        cout << "\n" << string(42, '=') << endl;
        cout << "              INVOICE #" << id << endl;
        cout << string(42, '=') << endl;
        cout << left << setw(18) << " Item" << right << setw(5) << "Qty"
             << setw(9) << "Price" << setw(10) << "Total" << endl;
        cout << string(42, '-') << endl;

        for (int i = 0; i < (int)names.size(); i++)
            cout << left << setw(18) << (" " + names[i])
                 << right << setw(5) << qtys[i]
                 << setw(9) << prices[i]
                 << setw(10) << prices[i] * qtys[i] << endl;

        cout << string(42, '-') << endl;
        cout << left << setw(32) << " Subtotal:" << right << setw(10) << subtotal() << endl;
        if (discountRate > 0)
            cout << left << setw(32) << " Discount:" << right << setw(10) << -discount() << endl;
        cout << left << setw(32) << " Tax:" << right << setw(10) << tax() << endl;
        cout << string(42, '=') << endl;
        cout << left << setw(32) << " TOTAL:" << right << setw(10) << total() << endl;
        cout << string(42, '=') << endl;
        cout << "      Thank you for visiting us!" << endl;
    }
};

// ============ Restaurant ============
class Restaurant {
private:
    string name;
    vector<Ingredient*> stock;
    vector<MenuItem*> menu;
    vector<Invoice*> orders;          // NEW: all saved orders
    int nextId = 1;                   // NEW: next order number
    double sales = 0, cost = 0, discounts = 0;
    int invoices = 0;

    // ---- helpers ----
    int findItem(string n) const {
        for (int i = 0; i < (int)menu.size(); i++)
            if (menu[i]->getName() == n) return i;
        return -1;
    }
    int findOrder(int id) const {
        for (int i = 0; i < (int)orders.size(); i++)
            if (orders[i]->getId() == id) return i;
        return -1;
    }
    double invoiceCost(Invoice* inv) const {
        double c = 0;
        for (int i = 0; i < inv->getLinesCount(); i++)
            c += menu[findItem(inv->getName(i))]->getCost() * inv->getQty(i);
        return c;
    }
    void addFinancials(Invoice* inv) {
        sales += inv->subtotal() - inv->discount();
        discounts += inv->discount();
        cost += invoiceCost(inv);
    }
    void removeFinancials(Invoice* inv) {
        sales -= inv->subtotal() - inv->discount();
        discounts -= inv->discount();
        cost -= invoiceCost(inv);
    }
public:
    Restaurant(string n) : name(n) {}
    ~Restaurant() {
        for (int i = 0; i < (int)orders.size(); i++) delete orders[i];
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
        cout << "\n" << string(55, '=') << endl;
        cout << "              " << name << " MENU" << endl;
        cout << string(55, '=') << endl;
        cout << " No  Item               Price     Ingredients" << endl;
        cout << string(55, '-') << endl;
        for (int i = 0; i < (int)menu.size(); i++) menu[i]->print(i + 1);
        cout << string(55, '=') << endl;
    }
    void showStock() const {
        cout << "\n------- STOCK -------\n";
        for (auto s : stock)
            cout << left << setw(15) << s->getName() << s->getQty() << endl;
    }

    // ---------- NEW ORDER ----------
    void takeOrder() {
        Invoice* inv = new Invoice(nextId);      // number is given by the system
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
                inv->addLine(menu[c - 1]->getName(), q, menu[c - 1]->getPrice());
            }
            cout << "Add more? (y/n): "; cin >> more;
        } while (more == 'y' || more == 'Y');

        if (inv->isEmpty()) {
            delete inv;
            cout << "Order cancelled.\n";
            return;
        }

        if (inv->subtotal() >= 50) {
            char d;
            cout << "Eligible for 10% discount. Apply? (y/n): "; cin >> d;
            if (d == 'y' || d == 'Y') inv->setDiscountRate(0.10);
        }
        inv->print();
        orders.push_back(inv);                   // NEW: order is saved
        nextId++;                                // number is used only when saved
        addFinancials(inv);
        invoices++;
    }

    // ---------- NEW: ORDERS LIST ----------
    void showOrders() const {
        if (orders.empty()) { cout << "\nNo saved orders.\n"; return; }
        cout << fixed << setprecision(2);
        cout << "\n" << string(36, '=') << endl;
        cout << " Order No   Items       Total" << endl;
        cout << string(36, '-') << endl;
        for (int i = 0; i < (int)orders.size(); i++)
            cout << " " << left << setw(10) << orders[i]->getId()
                 << setw(8) << orders[i]->getLinesCount()
                 << right << setw(12) << orders[i]->total() << endl;
        cout << string(36, '=') << endl;
    }

    // ---------- NEW: VIEW ONE ORDER ----------
    void viewOrder() const {
        int id;
        cout << "Order number: "; cin >> id;
        int idx = findOrder(id);
        if (idx == -1) { cout << "Order not found.\n"; return; }
        orders[idx]->print();
    }

    // ---------- NEW: CANCEL ORDER ----------
    void cancelOrder() {
        int id;
        cout << "Order number to cancel: "; cin >> id;
        int idx = findOrder(id);
        if (idx == -1) { cout << "Order not found.\n"; return; }
        Invoice* inv = orders[idx];

        removeFinancials(inv);
        for (int i = 0; i < inv->getLinesCount(); i++)
            menu[findItem(inv->getName(i))]->cancel(inv->getQty(i));   // stock comes back
        invoices--;
        delete inv;
        orders.erase(orders.begin() + idx);
        cout << "Order #" << id << " cancelled.\n";
    }

    // ---------- NEW: EDIT ORDER ----------
    void editOrder() {
        int id;
        cout << "Order number to edit: "; cin >> id;
        int idx = findOrder(id);
        if (idx == -1) { cout << "Order not found.\n"; return; }
        Invoice* inv = orders[idx];

        removeFinancials(inv);        // take the old order out of today's totals
        int ch;
        do {
            inv->print();
            cout << "\n1.Add item 2.Change quantity 3.Remove item 0.Done\nChoice: ";
            cin >> ch;

            if (ch == 1) {
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
                    inv->addLine(menu[c - 1]->getName(), q, menu[c - 1]->getPrice());
                }
            }
            else if (ch == 2) {
                int n, q;
                cout << "Line number: ";   cin >> n;
                cout << "New quantity: ";  cin >> q;
                if (n < 1 || n > inv->getLinesCount() || q <= 0)
                    cout << "Invalid input.\n";
                else {
                    MenuItem* m = menu[findItem(inv->getName(n - 1))];
                    int diff = q - inv->getQty(n - 1);
                    if (diff > 0 && !m->canPrepare(diff))
                        cout << "Not enough ingredients!\n";
                    else {
                        if (diff > 0) m->prepare(diff);
                        else if (diff < 0) m->cancel(-diff);
                        inv->setQty(n - 1, q);
                    }
                }
            }
            else if (ch == 3) {
                int n;
                cout << "Line number: "; cin >> n;
                if (n < 1 || n > inv->getLinesCount())
                    cout << "Invalid input.\n";
                else {
                    menu[findItem(inv->getName(n - 1))]->cancel(inv->getQty(n - 1));
                    inv->removeLine(n - 1);
                }
            }
        } while (ch != 0 && !inv->isEmpty());

        if (inv->isEmpty()) {         // everything removed = order cancelled
            delete inv;
            orders.erase(orders.begin() + idx);
            invoices--;
            cout << "Order is empty, so it was cancelled.\n";
            return;
        }

        // discount rule is checked again after the changes
        if (inv->subtotal() < 50) inv->setDiscountRate(0);
        else if (inv->getDiscountRate() == 0) {
            char d;
            cout << "Eligible for 10% discount. Apply? (y/n): "; cin >> d;
            if (d == 'y' || d == 'Y') inv->setDiscountRate(0.10);
        }
        addFinancials(inv);           // put the updated order back in the totals
        cout << "Order updated.";
        inv->print();
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
        cout << "\n1.Menu  2.New order  3.Orders list  4.View order\n"
             << "5.Edit order  6.Cancel order  7.Stock  8.Report  0.Exit\nChoice: ";
        cin >> ch;
        switch (ch) {
            case 1: r.showMenu(); break;
            case 2: r.takeOrder(); break;
            case 3: r.showOrders(); break;
            case 4: r.viewOrder(); break;
            case 5: r.editOrder(); break;
            case 6: r.cancelOrder(); break;
            case 7: r.showStock(); break;
            case 8: r.report(); break;
        }
    } while (ch != 0);
}
