# Pain Compiler Condemnation (PCC)

Análise Sintática:

	- Expressões matemáticas e lógicas  X
		. Operações Binárias (soma, subtração, multiplicação, divisão, módulo)  X
		. Operações Lógicas (AND, OR, >, <)  X
		. Precedência (Realizar operações na ordem correta)  X

	- Controle de Fluxo
		. Ifs e Switches  X
		. Ler abertura de parênteses, expressão e fechamento de parênteses		X
		. Operador ternário

		. For, While e Do While  X
		. GOTO e Break

	- Estruturas de Dados
		. Vetores e Matrizes
		. Encadeamento de colchetes (matriz[x][y][z])

	- Tipos definidos pelo usuário
		. Struct (permitindo variáveis internas e funções)
		. Estruturas de chave-valor (Dicionários/Mapas)

	- Funções
		. Declarar Função (Nome, Parâmetros, Bloco de Código)
		. Chamar Função (Passar argumentos)
		. Return

Análise Semântica:

	- Tabela de Símbolos: Classe que atua como memória do compilador que armazena os tipos, nomes e escopos das variáveis durante compilação
	- Verificação de Declaração Prévia: Ver se uma variável chamada já foi declarada em algum momento
	- Checagem de Tipos: Validar legalidade de operações e retornos

Transpilador:

	- Gerador de Código: Módulo que recebe a AST já validade pela semântica
	- Design Pattern Visitor: Recebe os nós em ordem e traduz para texto em um arquivo .c
	- Tradução de Estruturas Complexas: Mapear como structs são geradas em C e implementar hash tables
	- Linking e Loading: Fazer o g++ ser invocado automaticamente para já ter um arquivo executável no fim

