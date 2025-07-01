#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <cmath>
#include <algorithm> // Para std::sort na Busca Binária
 #include "json.hpp" // Certifique-se de que este cabeçalho está acessível

// Usando namespace para facilitar
using namespace std;
using namespace std::chrono;
 using json = nlohmann::json; // Descomente se estiver usando o nlohmann/json

// --- Estrutura Gema (sem alterações significativas) ---
struct Gema {
    int id;
    string cor;
    int intensidade;
    bool ocupado; // Indica se a posição está ocupada por uma gema válida

    Gema() : id(0), cor(""), intensidade(0), ocupado(false) {}
    Gema(int _id, string _cor) : id(_id), cor(_cor), ocupado(true) {
        intensidade = calcularIntensidade(_cor);
    }

    int calcularIntensidade(const string& hexColor) {
        if (hexColor.length() != 7 || hexColor[0] != '#') return 0;

        int vermelho = stoi(hexColor.substr(1, 2), nullptr, 16);
        int verde = stoi(hexColor.substr(3, 2), nullptr, 16);
        int azul = stoi(hexColor.substr(5, 2), nullptr, 16);

        return calculaLuminancia(vermelho, verde, azul);
    }

    int calculaLuminancia(int vermelho, int verde, int azul){
        return (int)(0.299 * vermelho + 0.587 * verde + 0.114 * azul);
    }
    
    // Sobrecarga do operador de igualdade para Gema (útil em testes ou buscas)
    bool operator==(const Gema& outra) const {
        return id == outra.id && cor == outra.cor && intensidade == outra.intensidade;
    }
};

class TabelaHash {
private:
    vector<Gema> tabela; 
    int capacity;        
    int totalElementos;
    int tentativasResolucao;

    const double MAX_LOAD_FACTOR = 0.7; // Manter os 70%

    int getNextPrime(int n) {
        while (true) {
            bool isPrime = true;
            if (n <= 1) isPrime = false;
            for (int i = 2; i * i <= n; ++i) {
                if (n % i == 0) {
                    isPrime = false;
                    break;
                }
            }
            if (isPrime) return n;
            n++;
        }
    }

    void rehash() {
        int oldCapacity = capacity;
        capacity = getNextPrime(oldCapacity * 2); // Dobra a capacidade e encontra o próximo primo
        
        vector<Gema> oldTable = move(tabela); // Move os dados para uma tabela temporária
        tabela.assign(capacity, Gema());       // Redimensiona a tabela principal e inicializa com Gema padrão
        
        totalElementos = 0; // Resetar para reinserir
        tentativasResolucao = 0; // Resetar estatísticas de colisão para o rehash
        
        // Reinserir todas as gemas da tabela antiga na nova tabela maior
        for (int i = 0; i < oldCapacity; ++i) {
            if (oldTable[i].ocupado) {
                // Não precisamos verificar o fator de carga aqui, pois já estamos redimensionando
                // E a gema deve ser inserida, pois sabemos que há espaço suficiente.
                int indiceOriginal = hashId(oldTable[i].id); // Usamos hashId como exemplo
                int indice = encontrarPosicaoLivre(indiceOriginal);
                tabela[indice] = oldTable[i];
                tabela[indice].ocupado = true; // Garantir que está como ocupado
                totalElementos++;
            }
        }
        cout << "Tabela hash redimensionada de " << oldCapacity << " para " << capacity << " posições." << endl;
    }

public:
    TabelaHash(int initialCapacity = 211) : totalElementos(0), tentativasResolucao(0) {
        if (initialCapacity <= 0) {
            throw invalid_argument("Capacidade inicial deve ser positiva.");
        }
        capacity = getNextPrime(initialCapacity); 
        tabela.assign(capacity, Gema()); 
    }
    
    int hashId(int id) {
        return abs(id) % capacity;
    }
    
    int hashCor(const string& cor) {
        int soma = 0;
        for (char c : cor) {
            soma += (int)c;
        }
        return soma % capacity;
    }
    
    int hashIntensidade(int intensidade) {
        return intensidade % capacity;
    }
    
    int encontrarPosicaoLivre(int indiceInicial) {
        int indice = indiceInicial;
        int tentativas = 0;
        
        while (tabela[indice].ocupado && tentativas < capacity) { 
            indice = (indice + 1) % capacity; 
            tentativas++;
            tentativasResolucao++;
        }
        
        if (tentativas >= capacity) {
            return -1;
        }
        
        return indice;
    }
    
    bool inserirPorId(const Gema& gema) {
        if ((double)(totalElementos + 1) / capacity >= MAX_LOAD_FACTOR) {
            rehash(); // redmenisonar
        }
        
        int indiceOriginal = hashId(gema.id);
        int indice = encontrarPosicaoLivre(indiceOriginal);
        
        if (indice == -1) {
            cout << "Erro crítico: Tabela cheia mesmo após rehash!" << endl;
            return false;
        }
        
      
        
        tabela[indice] = gema;  //copia para uma posicao
        tabela[indice].ocupado = true; 
        totalElementos++;
        
        if (indice != indiceOriginal) {
            // cout << "Colisão resolvida: ID " << gema.id 
            //      << " movido de posição " << indiceOriginal 
            //      << " para " << indice << endl;
        }
        
        return true;
    }
    
    Gema* buscarPorId(int id, int& passos) {
        passos = 0;
        int indiceOriginal = hashId(id);
        int indice = indiceOriginal;
        
        do {
            passos++;
            if (tabela[indice].ocupado && tabela[indice].id == id) {
                return &tabela[indice];
            }
            if (!tabela[indice].ocupado) {
                return nullptr;
            }
            indice = (indice + 1) % capacity; 
        } while (indice != indiceOriginal && passos < capacity);
        
        return nullptr;
    }
    
 
    vector<Gema*> buscarPorCor(const string& cor, int& passos) {
        passos = 0;
        vector<Gema*> resultado;
        for (int i = 0; i < capacity; i++) {
            passos++;
            if (tabela[i].ocupado && tabela[i].cor == cor) {
                resultado.push_back(&tabela[i]);
            }
        }
        return resultado;
    }
    
    vector<Gema*> buscarPorIntensidade(int intensidade, int& passos) {
        passos = 0;
        vector<Gema*> resultado; 
            passos++;
            if (tabela[i].ocupado && tabela[i].intensidade == intensidade) {
                resultado.push_back(&tabela[i]);
            }
        }
        return resultado;
    }
    
    bool atualizarCor(int id, const string& novaCor, int& passos) {
        Gema* gema = buscarPorId(id, passos);
        if (gema != nullptr) {
            gema->cor = novaCor;
            gema->intensidade = gema->calcularIntensidade(novaCor);
            return true;
        }
        return false;
    }
    
    void mostrarTabela() {
        cout << "\n=== Estado da Tabela Hash ===" << endl;
        int count = 0;
        for (int i = 0; i < capacity; i++) { 
            if (tabela[i].ocupado) {
                cout << "Posição " << i << ": ID=" << tabela[i].id 
                     << ", Cor=" << tabela[i].cor 
                     << ", Intensidade=" << tabela[i].intensidade << endl;
                count++;
                if (count >= 10 && capacity > 10) { 
                    cout << "... (mostrando apenas primeiras 10 de " << totalElementos << " elementos)" << endl;
                    break;
                }
            }
        }
    }
    
    void mostrarEstatisticas() {
        cout << "=== Estatísticas da Tabela Hash ===" << endl;
        cout << "Total de elementos: " << totalElementos << endl;
        cout << "Tamanho da tabela (capacidade): " << capacity << endl;
        cout << "Fator de carga: " << (double)totalElementos / capacity * 100 << "%" << endl;
        cout << "Tentativas de resolução de colisões (desde o último rehash/construção): " << tentativasResolucao << endl;
        cout << "Posições vazias: " << (capacity - totalElementos) << endl;
    }
};
class BuscaBinaria {
private:
    vector<Gema> gemasOrdenadas;
    
public:
    void ordenarPorId(vector<Gema>& gemas) {
        gemasOrdenadas = gemas;
        sort(gemasOrdenadas.begin(), gemasOrdenadas.end(), 
             [](const Gema& a, const Gema& b) { return a.id < b.id; });
    }
    
    Gema* buscarPorId(int id, int& passos) {
        passos = 0;
        int esquerda = 0;
        int direita = gemasOrdenadas.size() - 1;
        
        while (esquerda <= direita) {
            passos++;
            int meio = (esquerda + direita) / 2;
            
            if (gemasOrdenadas[meio].id == id) {
                return &gemasOrdenadas[meio];
            } else if (gemasOrdenadas[meio].id < id) {
                esquerda = meio + 1;
            } else {
                direita = meio - 1;
            }
        }
        
        return nullptr;
    }
    
    vector<Gema*> buscarPorCor(const string& cor, int& passos) {
        passos = 0;
        vector<Gema*> resultado;
        
        for (auto& gema : gemasOrdenadas) {
            passos++;
            if (gema.cor == cor) {
                resultado.push_back(&gema);
            }
        }
        
        return resultado;
    }
};

class CodexGemasComJSON {
private:
    vector<Gema> gemas;
    TabelaHash tabelaIds;
    TabelaHash tabelaCores;
    TabelaHash tabelaIntensidades;
    BuscaBinaria buscaBinaria;
    
    
public:
    void carregarGemas(const string& nomeArquivo) {
        ifstream arquivo(nomeArquivo);
        if (!arquivo.is_open()) {
            cout << "Erro ao abrir arquivo: " << nomeArquivo << endl;
            cout << "Certifique-se de que o arquivo 'gemas.jsonl' está no mesmo diretório." << endl;
            return;
        }
        
        string linha;
        int linhaNumero = 1;
        
        cout << "Carregando gemas do arquivo JSONL..." << endl;
        
        while (getline(arquivo, linha)) {
            if (linha.empty()) continue;
            
            try {
                json j = json::parse(linha);
                
                int id = j["id"];
                string cor = j["cor"];
                
                Gema novaGema(id, cor);
                gemas.push_back(novaGema);
                
                cout << "Linha " << linhaNumero << ": ID=" << id << ", Cor=" << cor << endl;
                
            } catch (const exception& e) {
                cout << "Erro ao processar linha " << linhaNumero << ": " << e.what() << endl;
                cout << "Linha problemática: " << linha << endl;
            }
            
            linhaNumero++;
        }
        
        arquivo.close();
        
        cout << "\nInserindo " << gemas.size() << " gemas nas tabelas hash..." << endl;
        
        for (const auto& gema : gemas) {
            tabelaIds.inserirPorId(gema);
            Gema gemaCor = gema;
            Gema gemaIntensidade = gema;
            tabelaCores.inserirPorId(gemaCor); 
            tabelaIntensidades.inserirPorId(gemaIntensidade);
        }
        
        cout << "Carregamento concluído!" << endl;
    }
    
    void buscarPorId(int id) {
        cout << "\n=== Busca por ID: " << id << " ===" << endl;
        
        int passos = 0;
        auto inicio = high_resolution_clock::now();
        Gema* resultado = tabelaIds.buscarPorId(id, passos);
        auto fim = high_resolution_clock::now();
        auto tempo = duration_cast<microseconds>(fim - inicio);
        
        if (resultado != nullptr) {
            cout << "Gema encontrada!" << endl;
            cout << "ID: " << resultado->id << endl;
            cout << "Cor: " << resultado->cor << endl;
            cout << "Intensidade: " << resultado->intensidade << endl;
        } else {
            cout << "Gema não encontrada!" << endl;
        }
        int passosBinaria = 0;
        auto inicioBinaria = high_resolution_clock::now();
        Gema* resultadoBinaria = buscaBinaria.buscarPorId(id, passosBinaria);
        auto fimBinaria = high_resolution_clock::now();
        auto tempoBinaria = duration_cast<nanoseconds>(fimBinaria - inicioBinaria);


        cout << "Busca binária: " << tempoBinaria.count() << " microssegundos" << endl;
        cout << "Passos binária necessários: " << passosBinaria << endl;
        cout << "Tempo Hash: " << tempo.count() << " microssegundos" << endl;
        cout << "Passos Hash necessários: " << passos << endl;
    }
    
    void buscarPorCor(const string& cor) {
        cout << "\n=== Busca por Cor: " << cor << " ===" << endl;
        
        int passos = 0;
        auto inicio = high_resolution_clock::now();
        vector<Gema*> resultado = tabelaCores.buscarPorCor(cor, passos);
        auto fim = high_resolution_clock::now();
        auto tempo = duration_cast<microseconds>(fim - inicio);
        
        cout << "Gemas encontradas: " << resultado.size() << endl;
        for (auto gema : resultado) {
            cout << "ID: " << gema->id << ", Cor: " << gema->cor 
                 << ", Intensidade: " << gema->intensidade << endl;
        }
        
        cout << "Tempo: " << tempo.count() << " microssegundos" << endl;
        cout << "Passos necessários: " << passos << endl;
    }
    
    void buscarPorIntensidade(int intensidade) {
        cout << "\n=== Busca por Intensidade: " << intensidade << " ===" << endl;
        
        int passos = 0;
        auto inicio = high_resolution_clock::now();
        vector<Gema*> resultado = tabelaIntensidades.buscarPorIntensidade(intensidade, passos);
        auto fim = high_resolution_clock::now();
        auto tempo = duration_cast<microseconds>(fim - inicio);
        
        cout << "Gemas encontradas: " << resultado.size() << endl;
        for (auto gema : resultado) {
            cout << "ID: " << gema->id << ", Cor: " << gema->cor 
                 << ", Intensidade: " << gema->intensidade << endl;
        }
        
        cout << "Tempo: " << tempo.count() << " microssegundos" << endl;
        cout << "Passos necessários: " << passos << endl;
    }
    string geraCorHexaAleatoria(){
        srand(time(nullptr));
        char novaCor[8];
        return  (string) sprintf(novaCor, "#%02X%02X%02X", rand() % 256, rand() % 256, rand() % 256);

    }
    void aplicarRaio(int id) {
        cout << "\n=== Aplicando Raio na Gema ID: " << id << " ===" << endl;
                
        string novaCor = geraCorAleatoria();
        int passos = 0;
        bool atualizado = tabelaIds.atualizarCor(id, string(novaCor), passos);
        
        if (atualizado) {
            cout << "Gema ID " << id << " teve sua cor alterada para " << novaCor << endl;
            cout << "Total de passos para atualização: " << passos << endl;
            
            int passosVerifica = 0;
            Gema* gemaAtualizada = tabelaIds.buscarPorId(id, passosVerifica);
            if (gemaAtualizada) {
                cout << "Nova intensidade: " << gemaAtualizada->intensidade << endl;
            }
        } else {
            cout << "Gema não encontrada!" << endl;
        }
    }
    
    void executarMenu() {
        int opcao;
        
        do {
            cout << "\n=== CODEX DAS GEMAS - SEM COLISÕES (JSONL) ===" << endl;
            cout << "1. Buscar gema por ID" << endl;
            cout << "2. Buscar gemas por cor" << endl;
            cout << "3. Buscar gemas por intensidade" << endl;
            cout << "4. Aplicar raio (alterar cor)" << endl;
            cout << "5. Mostrar estatísticas" << endl;
            cout << "6. Mostrar tabela (primeiras 10 gemas)" << endl;
            cout << "0. Sair" << endl;
            cout << "Escolha uma opção: ";
            cin >> opcao;
            
            switch (opcao) {
                case 1: {
                    int id;
                    cout << "Digite o ID da gema: ";
                    cin >> id;
                    buscarPorId(id);
                    break;
                }
                case 2: {
                    string cor;
                    cout << "Digite a cor (formato #RRGGBB): ";
                    cin >> cor;
                    buscarPorCor(cor);
                    break;
                }
                case 3: {
                    int intensidade;
                    cout << "Digite a intensidade (0-255): ";
                    cin >> intensidade;
                    buscarPorIntensidade(intensidade);
                    break;
                }
                case 4: {
                    int id;
                    cout << "Digite o ID da gema para aplicar raio: ";
                    cin >> id;
                    aplicarRaio(id);
                    break;
                }
                case 5:
                    cout << "\nEstatísticas da Tabela de IDs:" << endl;
                    tabelaIds.mostrarEstatisticas();
                    cout << "\nEstatísticas da Tabela de Cores:" << endl;
                    tabelaCores.mostrarEstatisticas();
                    cout << "\nEstatísticas da Tabela de Intensidades:" << endl;
                    tabelaIntensidades.mostrarEstatisticas();
                    break;
                case 6:
                    tabelaIds.mostrarTabela();
                    break;
                case 0:
                    cout << "Saindo do Codex..." << endl;
                    break;
                default:
                    cout << "Opção inválida!" << endl;
            }
        } while (opcao != 0);
    }
};

int main() {
    CodexGemasComJSON codex;
    
    cout << "=== CODEX DAS GEMAS - VERSÃO SEM COLISÕES ===" << endl;
    cout << "Lendo arquivo JSONL com nlohmann/json..." << endl;
    
    codex.carregarGemas("gemas.jsonl");
    
    codex.executarMenu();
    
    return 0;
}