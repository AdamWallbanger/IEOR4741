import sys, csv, collections
import matplotlib.pyplot as plt

def main():
    if len(sys.argv) < 3:
        print("Usage: python3 make_chart.py results.csv chart.png")
        sys.exit(1)
    csv_path, out_png = sys.argv[1], sys.argv[2]

    rows = []
    with open(csv_path, 'r', newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(row)

    data = collections.defaultdict(list)
    for r in rows:
        key = (r['pattern'], r['impl'])
        ops = float(r['ops_per_sec'])
        data[key].append(ops)

    patterns = ['homogeneous', 'mixed', 'bursty']
    impls = ['nonvirtual', 'virtual']  # order on chart

    med = {}
    for p in patterns:
        for im in impls:
            vals = sorted(data.get((p, im), []))
            if vals:
                m = vals[len(vals)//2] if len(vals)%2==1 else 0.5*(vals[len(vals)//2-1]+vals[len(vals)//2])
            else:
                m = 0.0
            med[(p,im)] = m

    x = range(len(patterns))
    width = 0.35

    fig = plt.figure(figsize=(7,4.5))
    nv = [med[(p,'nonvirtual')] for p in patterns]
    vv = [med[(p,'virtual')] for p in patterns]

    plt.bar([i - width/2 for i in x], nv, width, label='non-virtual')
    plt.bar([i + width/2 for i in x], vv, width, label='virtual')

    plt.xticks(list(x), patterns)
    plt.ylabel('Orders per second')
    plt.title('Virtual vs Non-virtual Throughput (median across repeats)')
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_png, dpi=150)
    print(f"Wrote {out_png}")

if __name__ == "__main__":
    main()
