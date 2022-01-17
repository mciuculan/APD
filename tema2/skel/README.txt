Ciuculan Maria
333 CC
			Tema 2
		Paradigma Map-Reduce

Pentru inceput, citesc fisierul initial. Am creat solutiile partiale din executor service
intr-o lista concurenta pentru a nu se suprapune solutiile. Impart fiecare fisier in
bucati aproape egale si trimit fiecarui worker ce are de facut.
Pentru task urile de Map, verific inceputul si finalul fragmentului. Daca ma aflu in
mijlocul cuvantului in oricare dintre cele doua situatii, modific atat inceputul cat si
finalul, astfel incat sa am cuvinte intregi.
Nu are sens sa mai verific nimic daca sunt deja la final de text motiv pt care folosesc
variabila finish. Apoi, impart dupa separatori si adaug intr-un hashmap cuvintele
cu numarul lor de aparitii. Apoi creez un dictionar in care adaug cuvinte nenule, cu
frecventele si lungimile respective. Salvez cel mai lung cuvand la final.
Dupa ce termin map ul pe fiecare fragment astept cu o bariera si cand este gata,
trec la urmatorul fisier.
Cand termin cu fisierele, combin solutiile partiale intr-o lista comula de dictionare,
in care, pentru fiecare fisier cunosc si lungimile si frecventele.
Apoi, pentru fiecare dictionar din lista comuna, pun cate un worker sa calculeze
rangurile, sa caute lungimile maximale si frecventele corespunzatoare pentru
acestea.
Apoi, sortez rezultatele finale descrescator dupa rangul lor si le printez.
