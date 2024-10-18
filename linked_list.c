#include "linked_list.h"


/*
 Initializes the linked list.
 param: head Pointer to a pointer to the head of the linked list.
 param: size Size of the memory pool to be created for memory management.
 This function initializes the memory manager and sets the head to NULL.
 */
void list_init(Node** head, size_t size) 
{
    mem_init(size * sizeof(Node)); // Initialize the memory manager
    *head = NULL;   // Start with an empty list
}


/*
 Inserts a new node at the end of the linked list.
 param: head Pointer to a pointer to the head of the linked list.
 param: data The data to be stored in the new node.
 note: This function does nothing if memory allocation fails.
 */
void list_insert(Node** head, uint16_t data) 
{
    if (!head) return; // Check if head is NULL

    Node* new_node = (Node*)mem_alloc(sizeof(Node));    // Allocate memory for the new node
    if (!new_node) return;                              // Exit if memory allocation fails

    new_node->data = data; // Set the data for the new node
    new_node->next = NULL; // Initialize the next pointer to NULL

    // if List is empty, set new node as the head
    if (*head == NULL) 
    {
        *head = new_node; // List is empty
    } 
    // else traverse to the end and add new node
    else 
    {
        Node* current_node = *head;             // Start from the head
        while (current_node->next)              // Traverse to the end
        {
            current_node = current_node->next;  // Traverse to the end
        }
        current_node->next = new_node;          // Add new node to the end
    }
}


/*
 Inserts a new node after a given node in the linked list.
 param: prev_node Pointer to the node after which the new node will be inserted.
 param: data The data to be stored in the new node.
 note: This function does nothing if prev_node is NULL or memory allocation fails.
 */
void list_insert_after(Node* prev_node, uint16_t data) 
{
    if (!prev_node) return; // Check if the previous node is NULL

    Node* new_node = (Node*)mem_alloc(sizeof(Node));  // Allocate memory for the new node
    if (!new_node) return;                            // Exit if memory allocation fails

    new_node->data = data;               // Set the data for the new node
    new_node->next = prev_node->next;    // Insert new node after prev_node
    prev_node->next = new_node;          // Insert new node after prev_node
}


/*
 Inserts a new node before a given node in the linked list.
 param: head Pointer to a pointer to the head of the linked list.
 param: next_node Pointer to the node before which the new node will be inserted.
 param: data The data to be stored in the new node.
 note: This function does nothing if head or next_node is NULL or if insertion fails.
 */
void list_insert_before(Node** head, Node* next_node, uint16_t data) 
{
    if (!head || !next_node) return; // Check if head or next_node is NULL

    Node* new_node = (Node*)mem_alloc(sizeof(Node)); // Allocate memory for the new node
    if (!new_node) return;                           // Exit if memory allocation fails

    new_node->data = data; // Set the data for the new node

    // Insert at the beginning if next_node is the head
    if (*head == next_node) 
    {
        new_node->next = *head; // New node becomes the head
        *head = new_node;       // Update the head pointer
    } 
    // Traverse to find the node before next_node 
    else 
    {
        Node* current_node = *head;  // Traverse to find the node before next node

        // Find the node before next node
        while (current_node && current_node->next != next_node)
        {
            current_node = current_node->next; // Find the node before next node
        }
        // Insert the new node before next node
        if (current_node) 
        {
            new_node->next = current_node->next;  // Link new node to next node
            current_node->next = new_node;        // Insert before next node
        }
        // Free memory if insertion point not found
        else 
        {
            mem_free(new_node); // Free if insertion point not found
        }
    }
}


/*
 Deletes a node with the specified data from the linked list.
 param: head Pointer to a pointer to the head of the linked list.
 param: data The data of the node to be deleted.
 note: This function does nothing if head is NULL or the list is empty.
 */
void list_delete(Node** head, uint16_t data) 
{
    if (!head || !*head) return;    // Check if head or list is empty

    Node* current_node = *head;         // Start from the head
    Node* prev_node = NULL;             // Initialize the previous node

    // Traverse to find the node with the specified data
    while (current_node && current_node->data != data) 
    {
        prev_node = current_node;           // Track the previous node
        current_node = current_node->next;  // Traverse to find the node
    }

    // If the node is found, delete it
    if (current_node) 
    {
        // Link the previous node to the next node
        if (prev_node) 
        {
            prev_node->next = current_node->next; // Unlink the node
        }
        // Update head if needed
        else 
        {
            *head = current_node->next; // Update head if needed
        }

        mem_free(current_node); // Free memory of the deleted node
    }
}


/*
 Searches for a node with the specified data in the linked list.
 param: head Pointer to a pointer to the head of the linked list.
 param: data The data of the node to be searched.
 return: Pointer to the found node, or NULL if the node is not found.
 */
Node* list_search(Node** head, uint16_t data) 
{
    if (!head) return NULL; // Check if head is NULL

    Node* current_node = *head;  // Start from the head

    // Traverse to find the node with the specified data
    while (current_node) 
    {
        if (current_node->data == data) return current_node;    // Node found
        current_node = current_node->next;                      // Move to the next node
    }

    return NULL; // Node not found
}


/*
 Displays the linked list.
 param: head Pointer to a pointer to the head of the linked list.
 note: Prints an empty list representation if the list is empty.
 */
void list_display(Node** head)
 {
    // Check if head or list is empty
    if (!head || !*head) 
    {
        printf("[]"); // Print an empty list
        return;
    }

    Node* current_node = *head; // Start from the head
    printf("[");
    
    // Traverse the list and print the elements
    while (current_node) 
    {
        printf("%d", current_node->data);       // Print the data
        current_node = current_node->next;      // Move to the next node
        if (current_node) printf(", ");         // Print a comma if not the last element
    }

    printf("]");
}


/*
 Displays a range of nodes in the linked list from start_node to end_node.
 param: head Pointer to a pointer to the head of the linked list.
 param: start_node Pointer to the starting node of the range.
 param: end_node Pointer to the ending node of the range.
 note: Prints an empty list representation if the list is empty.
 */
void list_display_range(Node** head, Node* start_node, Node* end_node) 
{
    // Check if head or list is empty
    if (!head || !*head) 
    {
        printf("[]"); // Print an empty list
        return;
    }

    Node* current_node = start_node ? start_node : *head; // Start from start_node or head

    printf("[");
    bool first_element = true;  // Flag to check if it's the first element

    // Traverse the list and print the elements
    while (current_node) 
    {
        if (!first_element) 
        {
            printf(", "); // Print a comma for subsequent elements
        } 
        else 
        {
            first_element = false; // The first element will not have a leading comma
        }

        printf("%d", current_node->data);           // Print the data
        if (current_node == end_node) break;        // Stop if we've reached the end_node
        current_node = current_node->next;          // Move to the next node
    }

    printf("]");
}


/*
 Counts the number of nodes in the linked list.
 param: head Pointer to a pointer to the head of the linked list.
 return: The number of nodes in the linked list.
 */
int list_count_nodes(Node** head) 
{
    if (!head) return 0;    // Check if head is NULL

    int node_count = 0;          // Initialize the count
    Node* current_node = *head;  // Start from the head
    
    // Traverse the list and count the nodes
    while (current_node) 
    {
        node_count++;                           // Increment the count
        current_node = current_node->next;      // Move to the next node
    }

    return node_count; // Return the node count
}


/*
 Cleans up the linked list and frees all nodes.
 param: head Pointer to a pointer to the head of the linked list.
 note: This function sets the head to NULL after freeing all nodes.
 */
void list_cleanup(Node** head) 
{
    if (!head) return; // Check if head is NULL

    Node* current_node = *head;  // Start from the head

    // Traverse the list and free all nodes
    while (current_node) 
    {
        Node* temp_node = current_node;       // Store the current node
        current_node = current_node->next;    // Move to the next node
        mem_free(temp_node);                  // Free memory for the current node
    }

    *head = NULL; // Reset the head pointer
}
