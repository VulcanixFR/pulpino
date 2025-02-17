#!/bin/bash
#fichier généré à l'aide de ChatGPT 4o

# Fonction pour commenter une ligne spécifique dans un fichier
comment_specific_line() {
    local file="$1"
    local search_pattern="$2"
    local comment_symbol="$3"

    # Créer un fichier temporaire pour stocker le contenu modifié
    local temp_file=$(mktemp)

    # Lire chaque ligne du fichier
    while IFS= read -r line; do
        # Vérifier si la ligne contient le motif de recherche et n'est pas déjà commentée
        if [[ "$line" == *"$search_pattern"* && "$line" != "$comment_symbol"* ]]; then
            echo "${comment_symbol} ${line}" >> "$temp_file"
        else
            echo "$line" >> "$temp_file"
        fi
    done < "$file"

    # Remplacer le fichier original par le fichier temporaire
    mv "$temp_file" "$file"
}

# Motif de recherche pour la ligne à commenter
search_pattern="define DIFT"

# Liste des fichiers à modifier
files_to_modify=(
    "$ROOT_DIR/ips/riscv/include/riscv_config.sv"
    "$ROOT_DIR/rtl/includes/config.sv"
    "$ROOT_DIR/sw/apps/buffer_overflow/mytest.c"
    "$ROOT_DIR/sw/apps/format_string/wu-ftpd.c"
    # Ajoute d'autres fichiers ici
)

# Parcourir chaque fichier et commenter la ligne spécifique
for file in "${files_to_modify[@]}"; do
    if [[ -f "$file" ]]; then
        case "$file" in
            *.c|*.h)
                comment_specific_line "$file" "$search_pattern" "//"
                ;;
            *.v|*.sv)
                comment_specific_line "$file" "$search_pattern" "//"
                ;;
        esac
    else
        echo "Le fichier $file n'existe pas."
    fi
done

echo "Les lignes spécifiques ont été commentées dans les fichiers spécifiés."