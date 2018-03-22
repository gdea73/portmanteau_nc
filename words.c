#include "words.h"
#include "letter.h"

struct dictionary *loadDict(char *filename) {
	struct dictionary *dict = malloc(sizeof(struct dictionary));
	char **words = NULL;
	char *w = NULL;
	size_t n = 0; // getline() buffer size -- 0 for default
	size_t n_chars = 0;
	size_t line_idx = 0; // e.g., running word count
	size_t dictsize = WORDSBS;
	FILE *fp = NULL;

	if (!(fp = fopen(filename, "r"))) {
		fprintf(stderr, "could not read file: \"%s\"", filename);
		exit(1);
	}

	// allocate WORDSBS NULL pointers first
	if (!(words = calloc(WORDSBS, sizeof(*words)))) {
		fprintf(stderr, "could not allocate sufficient dictionary memory");
		exit(1);
	}

	while ((n_chars = getline(&w, &n, fp)) != -1) {
		while (n_chars > 0 && (w[n_chars - 1] == '\n' || w[n_chars - 1] == '\r')) {
			// strip newlines and carriage returns
			w[--n_chars] = '\0';
		}
		// copy the recently-read word into the array for the dictionary
		words[line_idx++] = strdup(w);

		// add another memory block to the array if necessary
		if (line_idx == dictsize) {
			char **temp = realloc(words, dictsize * 2 * sizeof(*words));
			if (!temp) {
				fprintf(stderr, "could not expand dictionary memory");
				exit(-1);
			}
			words = temp;
			dictsize *= 2;
		}
	}

	if (fp) { fclose(fp); }
	if (w) { free(w); }

	dict->words = words;
	dict->length = line_idx;
	return dict;
}

void freeDict(struct dictionary *dict) {
	for (int i = 0; i < dict->length; i++) {
		free(dict->words[i]);
	}
	free(dict->words);
	free(dict);
}

uint8_t isValidWord(struct dictionary *dict, char *sz) {
	// printf("checking whether \"%s\" is a valid word\n", sz);
	return binSearch(dict->words, sz, 0, dict->length - 1);
}

uint8_t binSearch(char **words, char *query, size_t startIdx, size_t endIdx) {
	// printf("calling binSearch with range [%zu, %zu] on string: %s\n", startIdx, endIdx, query);
	if (endIdx - startIdx <= 1) {
		return (strcmp(query, words[endIdx]) == 0 || strcmp(query, words[startIdx]) == 0); }
	size_t midIdx = (startIdx + endIdx) / 2;
	char *med = words[midIdx];
	if (strcmp(query, med) > 0) {
		return binSearch(words, query, midIdx, endIdx);
	} else if (strcmp(query, med) < 0) {
		return binSearch(words, query, startIdx, midIdx);
	}
	return 1;
}

char *reverse(char *s) {
	size_t len = strlen(s);
	char *rev;
	if (!(rev = calloc(len, sizeof(char)))) {
		fprintf(stderr, "failed to calloc to reverse string %s", s);
	}
	for (int i = 0; i < len; i++) {
		rev[i] = s[(len - 1) - i];
	}
	return rev;
}

int wordScore(char *word) {
	int wordScore = 0;
	for (int i = 0; i < strlen(word); i++) {
		wordScore += pointValue(word[i]);
	}
	return wordScore * SCORE_MULTIPLIER;
}
