#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <algorithm>
#include <unordered_set>

using namespace std;

using Edge = pair<int, int>;

bool byla[1000][5001]; // byla[i][j] mówi czy w odległości i od 1. skrzyżowania
                       // wystapiła już wartość j energii w baku
class Graph {
public:
    int V; // liczba wierzchołków
    vector<vector<int>> adj_list; // listy sąsiedztwa
    int *pred; // pred[i] to nr poprzednika i-tego węzła (skąd do niego weszliśmy)
    int shortest_path; // najkrótsza ścieżka od 1. do ostatniego wierzchołka

    Graph(Edge edges[], int E, int V) { // O(E)
        this->V = V;
        shortest_path = -1;
        pred = new int[V];
        for (int i = 0; i < V; i++) {
            pred[i] = -1;
        }

        adj_list.resize(V);
        for (int i = 0; i < E; i++) {
            int first = edges[i].first - 1;
            int second = edges[i].second - 1;
            adj_list[first].push_back(second);
            adj_list[second].push_back(first);
        }
    }

    void BFS(int start, int last);
};

// klasyczny BFS - otrzymujemy najkrótszą ścieżkę z 1. do ostatniego skrzyżowania
// O(V + E)
void Graph::BFS(int start, int last) {
    bool visited[V];
    int dist[V];
    for (int i = 0; i < V; i++) {
        visited[i] = false;
        dist[i] = INT_MAX;
    }
    dist[start - 1] = 0;

    queue<int> q;
    q.push(start - 1);
    visited[start - 1] = true;
    dist[start - 1] = 0;

    while (!q.empty()) {
        int node = q.front();
        q.pop();
        if (node == last - 1) {
            shortest_path = dist[node] + 1;
        }

        for (int &iter : adj_list[node]) {
            if (!visited[iter]) {
                q.push(iter);
                visited[iter] = true;
                pred[iter] = node;
                dist[iter] = dist[pred[iter]] + 1;
            }
        }
    }
}

bool dozwolona(int energia, int poj, const unordered_set<int>& zabronione) {
    if (energia < 0 || energia > poj ||
        (zabronione.find(energia) != zabronione.end())) {
        return false;
    } else {
        return true;
    }
}

int przejedz(int odl, int akt_energ, int poj, int koszt, int energ[],
             const unordered_set<int> &zabronione, int dlugosc, vector<int> &gdzie_laduje);

int nie_laduj(int odl, int akt_energ, int poj, int koszt, int energ[],
              const unordered_set<int> &zabronione, int dlugosc, vector<int> &gdzie_laduje) {
    if (odl == dlugosc - 1) {
        return akt_energ;
    } else if (byla[odl][akt_energ]) {
        return -1;
    } else {
        byla[odl][akt_energ] = true;
        return przejedz(odl, akt_energ, poj, koszt, energ,
                        zabronione, dlugosc, gdzie_laduje);
    }
}

int laduj(int odl, int akt_energ, int poj, int koszt, int energ[],
          const unordered_set<int> &zabronione, int dlugosc, vector<int> &gdzie_laduje) {
    akt_energ += energ[odl];
    if (dozwolona(akt_energ, poj, zabronione)) {
        return nie_laduj(odl, akt_energ, poj, koszt, energ, zabronione, dlugosc, gdzie_laduje);
    } else {
        // ładowanie powoduje awarię
        return -1;
    }
}

int przejedz(int odl, int akt_energ, int poj, int koszt, int energ[],
             const unordered_set<int> &zabronione, int dlugosc, vector<int> &gdzie_laduje) {
    odl++;
    akt_energ -= koszt;
    if (akt_energ < 0) {
        // bateria rozładowana
        return -1;
    }

    vector<int> gdzie_laduje1;
    vector<int> gdzie_laduje2;
    int l = laduj(odl, akt_energ, poj, koszt, energ,
                  zabronione, dlugosc, gdzie_laduje1);
    int nl = nie_laduj(odl, akt_energ, poj, koszt, energ,
                       zabronione, dlugosc, gdzie_laduje2);
    if (l > nl) {
        gdzie_laduje = gdzie_laduje1;
        gdzie_laduje.push_back(odl);
        return l;
    } else {
        gdzie_laduje = gdzie_laduje2;
        return nl;
    }
}

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    // wczytywanie danych
    int poj, koszt, ile_zabr;
    cin >> poj >> koszt >> ile_zabr;
    int zabr;
    unordered_set<int> zabronione;
    for (int i = 0; i < ile_zabr; i++) {
        cin >> zabr;
        zabronione.insert(zabr);
    }

    int ile_skrzyz, ile_drog;
    cin >> ile_skrzyz >> ile_drog;
    Edge odc[ile_drog];
    int a, b;
    for (int i = 0; i < ile_drog; i++) {
        cin >> a >> b;
        odc[i] = {a, b};
    }
    int energ[ile_skrzyz];
    for (int i = 0; i < ile_skrzyz; i++) {
        cin >> energ[i];
    }
    // koniec wczytywania danych

    Graph graf(odc, ile_drog, ile_skrzyz); // O(ile_drog)
    graf.BFS(1, ile_skrzyz); // O(ile_skryz + ile_drog)

    int dlugosc = graf.shortest_path;
    if (dlugosc == -1) {
        cout << -1; // ostatnie skrzyżowanie nie ma połączenia z pierwszym
        return 0;
    }
    vector<int> gdzie_laduje;
    bool ladowania[dlugosc];
    int przejscie[dlugosc];

    int akt = ile_skrzyz - 1;
    for (int i = dlugosc - 1; i >= 0; i--) { // O(ile_skrzyz)
        przejscie[i] = akt + 1;
        akt = graf.pred[akt];
        ladowania[i] = false;
    }

    int kon_energ = przejedz(0, poj, poj, koszt, energ,
                             zabronione, dlugosc, gdzie_laduje); // O(ile_skrzyz * poj)
    if (kon_energ == -1) {
        cout << -1; // nie da się dojechać bez rozładowania lub awarii
        return 0;
    }

    size_t ile_ladowan = gdzie_laduje.size();
    for (auto &iter : gdzie_laduje) { // O(ile_skrzyz)
        ladowania[iter] = true;
    }

    // wypisywanie wyników - O(ile_skrzyz)
    cout << dlugosc << " ";
    cout << kon_energ << " ";
    cout << ile_ladowan << '\n';

    for (int i = 0; i < dlugosc; i++) {
        cout << przejscie[i] << " ";
    }
    cout << '\n';

    for (int i = 0; i < dlugosc; i++) {
        if (ladowania[i]) {
            cout << przejscie[i] << " ";
        }
    }

    return 0;
}
