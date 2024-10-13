/**
 * @file sign_mira_256_tree.c
 * @brief Implementation of tree related functions
 */

#include <stdio.h>
#include "tree.h"



/**
 * \fn void sign_mira_256_tree_expand(sign_mira_256_seed_tree_t tree, const uint8_t *salt)
 * \brief This function compute a seed tree by expanding its master seed.
 *
 * The leaves of the tree are stored in the array at position (tree[SIGN_MIRA_256_PARAM_N_MPC - 1])
 *
 * \param[out] tree sign_mira_256_seed_tree_t Representation of the tree with master seed at position tree[0]
 * \param[in] salt uint8_t* Salt used for the signature
 */
void sign_mira_256_tree_expand(sign_mira_256_seed_tree_t tree, const uint8_t *salt) {
  uint8_t domain_separator = DOMAIN_SEPARATOR_TREE;
  hash_sha3_ctx ctx;

  for(size_t i = 0; i < 3; i++) {
    size_t from = i;
    size_t to = i * 2 + 1;

    hash_sha3_init(&ctx);
    hash_sha3_absorb(&ctx, salt, 2 * SIGN_MIRA_256_SECURITY_BYTES);
    hash_sha3_absorb(&ctx, (const uint8_t *) &from, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, tree[from], SIGN_MIRA_256_SECURITY_BYTES);
    hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
    hash_sha3_finalize(tree[to], &ctx);
  }

  for(size_t i = 3; i < (SIGN_MIRA_256_PARAM_N_MPC - 1); i+=1) {
    size_t from = i;
    size_t to = i * 2 + 1;   

    hash_sha3_init(&ctx);
    hash_sha3_absorb(&ctx, salt, 2 * SIGN_MIRA_256_SECURITY_BYTES);
    hash_sha3_absorb(&ctx, (const uint8_t *)&from, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, tree[from], SIGN_MIRA_256_SECURITY_BYTES);
    hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
    hash_sha3_finalize(tree[to], &ctx);
  }
}


/**
 * \fn sign_mira_256_tree_expand_partial(sign_mira_256_seed_tree_t partial_tree, const sign_mira_256_seed_tree_node_t partial_tree_seeds[SIGN_MIRA_256_PARAM_N_MPC_LOG2],
 *                                        const uint8_t *salt,  uint16_t alpha)
 *
 * @brief This function expands a partial tree of seeds. The alpha-th leaf is set to zero.
 *
 * @param[out] partial_tree sign_mira_256_seed_tree_t Representation of the expanded partial seed tree
 * @param[in]  partial_tree_seeds sign_mira_256_seed_tree_node_t Representation of the seeds
 * @param[in]  salt uint8_t* Salt used in the signature
 * @param[in]  alpha uint16_t Missing leaf
 */
void sign_mira_256_tree_expand_partial(sign_mira_256_seed_tree_t partial_tree, const sign_mira_256_seed_tree_node_t partial_tree_seeds[SIGN_MIRA_256_PARAM_N_MPC_LOG2],
                                        const uint8_t *salt, uint16_t alpha) {

  uint8_t domain_separator = DOMAIN_SEPARATOR_TREE;
  hash_sha3_ctx ctx;

  for (size_t i = 0, l = 0, j = 0; i < 3; i++, j++) {
    size_t N = (1U << l);
    if (j >= N) {  // increment depth
      l++;
      j = 0;
    }
    size_t from = i;
    size_t to = i * 2 + 1;
    size_t missing = (alpha >> (SIGN_MIRA_256_PARAM_N_MPC_LOG2 - l));            // missing node for the depth l
    size_t is_right = (~alpha >> (SIGN_MIRA_256_PARAM_N_MPC_LOG2 - 1 - l)) & 1;  // position in the depth l + 1

    if (j == missing) {
      memcpy(partial_tree[to + is_right], partial_tree_seeds[l], SIGN_MIRA_256_SECURITY_BYTES);
    } else {
      hash_sha3_init(&ctx);
      hash_sha3_absorb(&ctx, salt, 2 * SIGN_MIRA_256_SECURITY_BYTES);
      hash_sha3_absorb(&ctx, (const uint8_t *) &from, sizeof(uint8_t));
      hash_sha3_absorb(&ctx, partial_tree[from], SIGN_MIRA_256_SECURITY_BYTES);
      hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
      hash_sha3_finalize(partial_tree[to], &ctx);
    }
  }

  for(size_t i = 3, l = 1, j = 2; i < (SIGN_MIRA_256_PARAM_N_MPC - 1); i++, j++) {
    size_t N = (1U << l);
    if (j >= N) {  // increment depth
      l++;
      j = 0;
    }
    size_t from = i;
    size_t to = i * 2 + 1;

    size_t missing = (alpha >> (SIGN_MIRA_256_PARAM_N_MPC_LOG2 - l));            // missing node for the depth l
    size_t is_right = (~alpha >> (SIGN_MIRA_256_PARAM_N_MPC_LOG2 - 1 - l)) & 1;  // position in the depth l + 1
    size_t times4i = (i - 3) & 0x3;    

    if (j == missing) {
        memcpy(partial_tree[to + is_right], partial_tree_seeds[l], SIGN_MIRA_256_SECURITY_BYTES);        
    } else {              
      hash_sha3_init(&ctx);
      hash_sha3_absorb(&ctx, salt, 2 * SIGN_MIRA_256_SECURITY_BYTES);
      hash_sha3_absorb(&ctx, (const uint8_t *)&from, sizeof(uint8_t));
      hash_sha3_absorb(&ctx, partial_tree[from], SIGN_MIRA_256_SECURITY_BYTES);
      hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
      hash_sha3_finalize(partial_tree[to], &ctx);
    
    }
  }
}

/**
 * \fn void sign_mira_256_tree_compute_partial(sign_mira_256_seed_tree_node_t partial_tree_seeds[SIGN_MIRA_256_PARAM_N_MPC_LOG2],
 *                                                const sign_mira_256_seed_tree_t tree, const uint16_t alpha)
 *
 * @brief This function returns the seed tree needed to compute all leaves except the alpha-th leaf
 *
 * @param[out] partial_tree_seeds Array of seeds needed to rebuild the tree with the missing alpha leaf
 * \param[in] tree sign_mira_256_seed_tree_t Representation of the tree with master seed at position tree[0]
 * @param[in] alpha uint16_t Missing leaf
 */
void sign_mira_256_tree_compute_partial(sign_mira_256_seed_tree_node_t partial_tree_seeds[SIGN_MIRA_256_PARAM_N_MPC_LOG2],
                                           const sign_mira_256_seed_tree_t tree, uint16_t alpha) {

    for (size_t i = 0; i < SIGN_MIRA_256_PARAM_N_MPC_LOG2; i++) {
        size_t depth = (1U << (i + 1U)) - 1;
        size_t node = (alpha >> (SIGN_MIRA_256_PARAM_N_MPC_LOG2 - 1U - i)) ^ 1U;
        memcpy(partial_tree_seeds[i], tree[depth + node], SIGN_MIRA_256_SECURITY_BYTES);
    }
}