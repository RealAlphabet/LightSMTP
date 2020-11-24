#include <mongoc/mongoc.h>

int main(int argc, char **argv)
{
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    bson_error_t error;
    bson_t *doc;

    mongoc_init();
    client = mongoc_client_new("mongodb://127.0.0.1/?appname=lumz-smtp");
    collection = mongoc_client_get_collection(client, "lumz-dev", "mails");

    doc = bson_new();
    BSON_APPEND_UTF8(doc, "from", "alphabet@lumzapp.com");
    BSON_APPEND_UTF8(doc, "to", "alphabet@lumzapp.com");
    BSON_APPEND_UTF8(doc, "mail", "JE TAIME DARKYKY");
    mongoc_collection_insert_one(collection, doc, NULL, NULL, &error);

    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
    mongoc_cleanup();

    return 0;
}
