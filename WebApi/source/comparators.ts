export function hash(value: any) {
  if(value.hash !== undefined) {
    return value.hash().toString();
  } else {
    return value.toString();
  }
}

export function equals(left: any, right: any) {
  if(left.equals !== undefined) {
    return left.equals(right);
  }
  return left === right;
}

/** Computes the hash of a string.
 * @param value - The string to hash.
 * @return The hash of the specified string.
 */
export function hashString(value: string): number {
  if(value.length === 0) {
    return 0;
  }
  let hash = 0;
  for(let i = 0; i < value.length; ++i) {
    hash = ((hash << 5) - hash) + value.charCodeAt(i);
    hash |= 0;
  }
  return hash;
}

/** Combines two hash values together.
 * @param seed - The initial hash value.
 * @param hash - The hash to combine with the seed.
 * @return The combined hash value.
 */
export function hashCombine(seed: number, hash: number): number {
  return seed ^ (hash + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}
